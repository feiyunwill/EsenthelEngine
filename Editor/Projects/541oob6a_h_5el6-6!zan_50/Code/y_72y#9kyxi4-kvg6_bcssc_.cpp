/******************************************************************************

   Image Mask can be used for example for drawing pregenerated world mini maps
      onto the circular mini map gui element on the screen.

/******************************************************************************/
ImagePtr logo, mask;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
bool Init()
{
   logo=UID(1119600675, 1212460399, 80010661, 526665178);
   mask=UID(2338406646, 1284858740, 3530955697, 2844784642);

   return true;
}
void Shut()
{
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   return true;
}
/******************************************************************************/
void Draw()
{
   D    .clear(TURQ);
   logo->drawMask(WHITE, TRANSPARENT, Rect_C(0, 0, 2, 1), *mask, Rect_C(Ms.pos(), 1.2, 1.2));
   D    .text(0, 0.9, "Please move the mouse around the screen");
}
/******************************************************************************/
