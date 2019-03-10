/******************************************************************************/
class BackgroundLoader
{
   // static functions
   static bool BackgroundLoad(Thread &thread) // this function will be called on a secondary thread to load the initial world data
   {
      ThreadMayUseGPUData(); // notify that the thread may use GPU data (required for OpenGL renderer)
      Game.World.update(Cam.at); // call the world update which will load the necessary data
      return false; // don't continue the thread
   }

   // members
   Thread thread;

   // methods
   void del() // delete background loader
   {
      Game.World.updateBreak(); // request break of the updating method
      thread.del();
   }
   void start() // start background loader
   {
      thread.create(BackgroundLoad); // start loader thread
   }
   bool update() // update background loader
   {
      if(!thread.created())return false; // not loading anything
      customUpdate(); // call custom updating method
      return true; // loading
   }
   bool draw() // draw background loader
   {
      if(!thread.created())return false; // not loading anything
      customDraw(); // call custom drawing method
      if(!thread.active()) // thread is created but finished processing
      {
         thread.del(); // delete the thread
         D.setFade(1.0); // enable screen fading from loading screen to the game
      }
      return true; // loading or finishing
   }

   void customUpdate() // you can modify this method and perform custom updating of the loading screen
   {
      Time.wait(1000/24); // limit main thread speed to 24 fps, to give more cpu power for background thread
   }
   void customDraw() // you can modify this method and perform custom drawing of the loading screen
   {
      D.clear(TURQ);
      D.text (0, 0, S+"Loading... ("+Round(Game.World.updateProgress()*100)+"%)");
   }
}
/******************************************************************************/
