/******************************************************************************/
void InitPre()
{
   EE_INIT();
   Ms.hide();
   Ms.clip(null, 1);
}
/******************************************************************************/
bool Init()
{
   Cam.dist=5; // set initial camera distance to 5 meters
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

   Cam.transformByMouse(0.1, 10, CAMH_ROT|CAMH_ZOOM); // simple camera handle allowing minimum 0.1 and maximum 10 meters zoom, and allowing rotation and zooming

   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(WHITE);

   SetMatrix(MatrixIdentity); // set matrix before drawing

              Box (1                    ).draw(BLACK); // draw black box with 1 meter radius
   if(Ms.b(0))Ball(1  ,    Vec(-3, 0, 0)).draw(BLACK); // when 0th mouse button pushed draw black ball with 1   meter radius and (-3,0,0) position
   if(Ms.b(1))Tube(0.3, 2, Vec( 3, 0, 0)).draw(BLACK); // when 1st mouse button pushed draw black tube with 0.1 meter radius, 2 meters height and (3,0,0) position
}
/******************************************************************************/
