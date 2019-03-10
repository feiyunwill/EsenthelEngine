/******************************************************************************/
Image image;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
Bool Init()
{
   image.create2D(256, 256, IMAGE_B8G8R8A8, 1); // create 256X256 image, B8G8R8A8 type, 1 mipmap

   if(image.lock()) // in order to edit the texture we must first lock it
   {
      FREPD(y, image.h()) // iterate through all y's
      FREPD(x, image.w()) // iterate through all x's
      {
         image.color(x, y, Color(x, y, 0, 255)); // set image color at (x,y) coordinates
      }
      image.unlock(); // unlock
   }
   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
Bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(WHITE);

   image.draw(Rect(-0.5, -0.5, 0.5, 0.5));
}
/******************************************************************************/
