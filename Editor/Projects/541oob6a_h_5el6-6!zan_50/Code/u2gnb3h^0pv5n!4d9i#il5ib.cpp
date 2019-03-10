/******************************************************************************/
Playlist  Battle     , // playlists
          Explore    ;
Sound     sound      ; // sound
Slider    vol_fx     , // volume bars
          vol_music  ,
          vol_ambient;
Text     tvol_fx     , // volume text
         tvol_music  ,
         tvol_ambient;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   D.scale(2);
}
/******************************************************************************/
bool Init()
{
   if(!Battle.songs()) // create Battle playlist
   {
      Battle+=UID(179898080, 1327326228, 2705071249, 4171399536); // add "battle0" track to 'Battle' playlist
      Battle+=UID(746976398, 1110313615, 3079654847, 358289912);  // add "battle1" track to 'Battle' playlist
   }
   if(!Explore.songs()) // create Explore playlist
   {
      Explore+=UID(2222101198, 1138675473, 1518921890, 1804050639); // add "explore" track to 'Explore' playlist
   }

   Music  .play(Battle ); // play playlist as music
   Ambient.play(Explore); // play playlist as ambient
   sound  .play(UID(96441856, 1299394573, 764242358, 3923793607), true); // play looped sound

   // create gui sliders
   Gui+=vol_fx     .create(Rect_L(-0.1,  0.2, 0.3, 0.1), SoundVolume.fx     ()); Gui+=tvol_fx     .create(Vec2(-0.3,  0.2), "Fx"     );
   Gui+=vol_music  .create(Rect_L(-0.1,  0.0, 0.3, 0.1), SoundVolume.music  ()); Gui+=tvol_music  .create(Vec2(-0.3,  0.0), "Music"  );
   Gui+=vol_ambient.create(Rect_L(-0.1, -0.2, 0.3, 0.1), SoundVolume.ambient()); Gui+=tvol_ambient.create(Vec2(-0.3, -0.2), "Ambient");

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
   Gui.update();

   // set new volumes
   SoundVolume.fx     (vol_fx     ());
   SoundVolume.music  (vol_music  ());
   SoundVolume.ambient(vol_ambient());
   return true;
}
/******************************************************************************/
void Draw()
{
   D  .clear(WHITE);
   Gui.draw ();
}
/******************************************************************************/
