/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Cam.pitch=0.5;
}
bool Init()
{
   Sky.atmospheric(); // sky

   // sun
   {
      Sun.image=UID(1275694243, 1199742097, 1108828586, 1055787228);
      Sun.pos=!Vec(1, 1, 1);
   }

   // moon
   {
      Astro &moon=Astros.New();            // create a new astronomical object in 'Astros' container
      moon.image=UID(1194910096, 1331950171, 278997914, 2918364784); // set image
      moon.pos =!Vec(-1, 1, 1);            // set custom position
      moon.size*=0.6;                      // decrease default size
      moon.blend=false;                    // disable blending, and thus set adding mode
      moon.image_color.set(64, 64, 64, 0); // set image color
      moon.light_color.zero();             // disable light casting
   }

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

   return true;
}
/******************************************************************************/
void Render()
{
}
void Draw()
{
   Renderer(Render);
}
/******************************************************************************/
