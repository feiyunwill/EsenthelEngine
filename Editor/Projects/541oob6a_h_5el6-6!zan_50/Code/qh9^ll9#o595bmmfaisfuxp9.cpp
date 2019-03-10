/******************************************************************************/
Mesh  box,
      ball;
Image image;
flt   density_factor=0.05;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
   D.ambientPower(0.1);
}
/******************************************************************************/
bool Init()
{
   Cam.dist=3;

   // create standard meshes and materials
   MaterialPtr material=UID(2123216029, 1141820639, 615850919, 3316401700);

   box .parts.New().base.create( Box(5), VTX_TEX0|VTX_NRM|VTX_TAN).reverse(); box .material(material).setRender().setBox();
   ball.parts.New().base.create(Ball(1), VTX_TEX0|VTX_NRM|VTX_TAN)          ; ball.material(material).setRender().setBox();

   // create holographic image
   int size=64;
   image.create(size, size, size, IMAGE_R8G8B8A8, IMAGE_3D, 1); // size x size x size dimensions, R8G8B8A8 type, 3D mode, 1 mip map
   if(image.lock()) // start manual pixel editing
   {
      REPD(z, image.d())
      REPD(y, image.h())
      REPD(x, image.w())
      {
         flt fx=flt(x)/(image.w()-1), // x pixel fraction value 0..1
             fy=flt(y)/(image.h()-1), // y pixel fraction value 0..1
             fz=flt(z)/(image.d()-1); // z pixel fraction value 0..1

         flt px=fx*2-1, // x pixel position value -1..1
             py=fy*2-1, // y pixel position value -1..1
             pz=fz*2-1; // z pixel position value -1..1

         Vec4 color=0;

         if(x<=1 || x>=image.w()-2
         || y<=1 || y>=image.h()-2
         || z<=1 || z>=image.d()-2) // if current pixel is on border
         {
            color.set(0, 1, 0, RandomF(0.5, 1)); // green color with random alpha
         }
         else // if current pixel is inside
         {
            // blue sphere
            flt length=Vec(px, py, pz).length();
            flt sphere=(1-Sat(Abs(length-0.5)*8));//*RandomF();

            // red fade
            flt red_fade=0.5*fx*fx*RandomF();

            // color value from blue sphere and red fade
            if(flt sum=sphere+red_fade) // total alpha value
               color.set(1*red_fade/sum, 0.5*sphere/sum, 1*sphere/sum, sum);
         }

         image.color3DF(x, y, z, color); // store the pixel color
      }
      image.unlock(); // end pixel editing
   }

   REP(3)image.sharpen(1, 1, true, true); // sharpen the image 3 times

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
   Cam.transformByMouse(0.1, 1000, CAMH_ZOOM|(Ms.b(1)?CAMH_MOVE:CAMH_ROT));

   if(Kb.b(KB_Q))density_factor/=1+Time.d();
   if(Kb.b(KB_W))density_factor*=1+Time.d();

   return true;
}
/******************************************************************************/
void Render()
{
   switch(Renderer())
   {
      case RM_PREPARE:
      {
         box .draw(MatrixIdentity);
         ball.draw(Matrix(Vec(0, -3, 0)));

         LightPoint(20, Vec(0, 3, 3)).add();
      }break;

      case RM_BLEND: // holographic images need to be drawn in RM_BLEND mode
      {
         image.drawVolume(WHITE, TRANSPARENT, OBox(Box(1)), density_factor, 1, 4, 64);
      }break;
   }
}
void Draw()
{
   Renderer(Render);
   D.text(0, 0.9, S+"Fps: "+Time.fps());
   D.text(0, 0.8, S+"Voxel Density Factor: "+density_factor+" (Q/W to change)");
}
/******************************************************************************/
