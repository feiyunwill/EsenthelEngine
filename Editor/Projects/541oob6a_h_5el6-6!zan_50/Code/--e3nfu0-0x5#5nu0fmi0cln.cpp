/******************************************************************************/
Sound sound;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC  ))return false;
   if(Kb.bp(KB_SPACE))SoundPlay(UID(3284313217, 1120923307, 106803632, 970034379)); // play sound on space

   if(Kb.bp(KB_ENTER)) // toggle water on enter
   {
      if(sound.playing()) // if already playing
      {
         sound.del(); // delete sound
      }else
      {
         sound.play(UID(96441856, 1299394573, 764242358, 3923793607), true); // play with loop option enabled
      }
   }

   // change volumes on mouse wheel
   {
      if(Ms.wheel()>0)sound.volume(sound.volume()+0.1);else
      if(Ms.wheel()<0)sound.volume(sound.volume()-0.1);
   }

   // change speed on LMB/RMB
   {
      if(Ms.b(0))sound.speed(sound.speed()-Time.d()*0.4);
      if(Ms.b(1))sound.speed(sound.speed()+Time.d()*0.4);
   }
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);
   D.text (0,  0.2,   "Press space to play 'metal'");
   D.text (0,  0.0, S+"Press enter to stop/play looped 'water' (playing: "     +sound.playing()+')');
   D.text (0, -0.2, S+"Use mouse wheel to change water sound volume (current: "+sound.volume ()+')');
   D.text (0, -0.4, S+  "Press LMB/RMB to change water sound speed (current: " +sound.speed  ()+')');
}
/******************************************************************************/
