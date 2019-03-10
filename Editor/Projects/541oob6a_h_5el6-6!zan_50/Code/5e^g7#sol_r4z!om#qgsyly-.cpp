/******************************************************************************

   This tutorial demonstrates displaying Esenthel Logo Animation.
   
   Unlike other tutorials, it does not load the assets from the Project Data,
      instead it tries to find the Editor's Assets path, and loads the assets from there.

   Esenthel Logo Animation - Video and Sound assets are located in the Editor's path:
      "Esenthel/Assets/Logo/Esenthel.webm" - video
      "Esenthel/Assets/Logo/Esenthel.ogg"  - sound

   You can use these files to include them in your own projects,
      and display the engine's logo in your own applications.

/******************************************************************************/
Video video;
Sound sound;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   D.mode(1280, 720);
}
/******************************************************************************/
bool Init()
{
   // find editor's assets path
   Str assets_path, engine_path=EE_ENGINE_PATH; if(!FullPath(engine_path))Exit("Please run this app directly from the Editor on the desktop");
   for(; engine_path.is(); engine_path=GetPath(engine_path))
   {
      Str temp=engine_path.tailSlash(true)+"Assets";
      if(FExistSystem(temp)){assets_path=temp; break;}
   }
   if(!assets_path.is())Exit("Can't find Editor's Assets path");
   assets_path+="/Logo/";

   // load assets
   if(!video.create(assets_path+"Esenthel.webm"))                Exit("Can't create video"); // create video from file
       sound.create(assets_path+"Esenthel.ogg" ); if(!sound.is())Exit("Can't create sound"); // create sound from file
   sound.play();

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

   video.update(sound.time()); // update video to sound time

   return true;
}
/******************************************************************************/
void Draw()
{
   D    .clear ();
   video.drawFs(); // draw video
}
/******************************************************************************/
