/******************************************************************************

   States are used for different "Application States"
   for example there can be "intro state", "menu state", "game state"

/******************************************************************************/
// INTRO
/******************************************************************************/
bool   InitIntro() {return true;}
void   ShutIntro() {}
bool UpdateIntro()
{
   if(Time.stateTime()>3 || Kb.bp(KB_ESC)) // if active state (which here is 'StateIntro') is running for more than 3 seconds or escape pressed
      StateMenu.set(1.0);                  // then switch to 'StateMenu' state with 1.0 second smooth fading
   return true;
}
void DrawIntro()
{
   D.clear(BLACK);
   D.text (0, 0, "Intro");
}
State StateIntro(UpdateIntro, DrawIntro, InitIntro, ShutIntro);
/******************************************************************************/
// MENU
/******************************************************************************/
bool   InitMenu() {return true;}
void   ShutMenu() {}
bool UpdateMenu()
{
   if(Kb.bp(KB_ESC))return false;         // when escape pressed exit application
   if(Kb.bp(KB_ENTER))StateGame.set(0.5); // when enter  pressed set 'StateGame' with 0.5 second smooth fading
   return true;
}
void DrawMenu()
{
   D.clear(GREY);
   D.text (0,  0  , "Menu");
   D.text (0, -0.3, "Press Enter to start the game");
   D.text (0, -0.5, "Press Escape to exit");
}
State StateMenu(UpdateMenu, DrawMenu, InitMenu, ShutMenu);
/******************************************************************************/
// GAME
/******************************************************************************/
bool   InitGame() {return true;}
void   ShutGame() {}
bool UpdateGame()
{
   if(Kb.bp(KB_ESC))StateMenu.set(1.0); // when escape pressed set 'StateMenu' with 1.0 second smooth fading
   return true;
}
void DrawGame()
{
   D.clear(TURQ);
   D.text (0, 0, "Game");
}
State StateGame(UpdateGame, DrawGame, InitGame, ShutGame);
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
bool Init()
{
   StateIntro.set(); // set StateIntro as the active state, this means that from now on, only StateIntro's update and drawing methods will be called instead of 'Main' and 'Draw'
   return true;
}
void Shut()
{
}
bool Update() {return false;} // unused
void Draw  () {             } // unused
/******************************************************************************

   Typically methods will be called in this order:
   
   Init()

       InitIntro()
         UpdateIntro()
           DrawIntro()
               ..
         UpdateIntro()
           DrawIntro()
       ShutIntro()

       InitMenu ()
         UpdateMenu()
           DrawMenu()
               ..
         UpdateMenu()
           DrawMenu()
       ShutMenu()

       InitGame()
         UpdateGame()
           DrawGame()
               ..
         UpdateGame()
           DrawGame()
       ShutGame()

       InitMenu()
         UpdateMenu()
           DrawMenu()
               ..
         UpdateMenu()
           DrawMenu()
       ShutMenu()
      
   Shut()

/******************************************************************************/
