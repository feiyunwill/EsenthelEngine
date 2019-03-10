/******************************************************************************/
class Cloud
{
   int cloud_index=Random(Elms(cloud_image));
   flt size=RandomF(0.8, 1.2);
   Vec pos;
   
   void draw()
   {
      cloud_image[cloud_index].draw(size, pos);
   }
}
/******************************************************************************/
VolumetricCloud cloud_image[8]; // cloud images
Memc<Cloud>     clouds; // cloud instances
Threads         threads;
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
   REPAO(cloud_image).createAsObject(Random(96, 160), 64, Random(96, 160), 0.03, &threads, Random());

   // volumetric images may not intersect with each other, so set positions based on a regular grid modified by slight offsets
   for(Int x=-2; x<=2; x++)
   for(Int z=-2; z<=2; z++)
      clouds.New().pos.set(x*3, 3, z*3)+=Random.vec(-0.2, 0.2);

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

   VolumetricCloud.Settings settings; settings.density=1;
   REPAO(cloud_image).update(settings);

   return true;
}
/******************************************************************************/
int Compare(C Cloud &a, C Cloud &b) {return CompareTransparencyOrderDist(a.pos, b.pos);}
void Render()
{
   switch(Renderer())
   {
      case RM_CLOUD: // clouds in order to affect the sun rays must be drawn in RM_CLOUD mode
      {
         // clouds are transparent so they need to be sorted before rendering
         clouds.sort(Compare);

         // draw the clouds
         REPAO(clouds).draw();
      }break;
   }
}
void Draw()
{
   Renderer(Render);
}
/******************************************************************************/
