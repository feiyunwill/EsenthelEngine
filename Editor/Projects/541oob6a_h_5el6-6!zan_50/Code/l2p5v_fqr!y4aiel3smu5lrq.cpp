/******************************************************************************/
Sound sound;        // sound
Vec   pos(0, 0, 3); // sound position
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
   sound.play(UID(96441856, 1299394573, 764242358, 3923793607), pos, 1, true); // play looped 3D sound, position='pos', volume=1, loop=true
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
   Cam.transformByMouse(0.1, 10, CAMH_ZOOM|CAMH_ROT);

   // update all 3D sound positions
   {
      sound.pos(pos); // since 'pos' is the same in each frame, the sound position doesn't need to be updated, but let's do it anyway
   }

   // update listener parameters
   {
      Listener.orn(Cam.matrix.z, Cam.matrix.y)  // set listener orientation (from camera)
              .pos(Cam.matrix.pos            ); // set listener position    (from camera)
   }
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(WHITE);

   SetMatrix();
   Ball(1, pos).draw(BLACK); // draw ball at sound position
}
/******************************************************************************/
