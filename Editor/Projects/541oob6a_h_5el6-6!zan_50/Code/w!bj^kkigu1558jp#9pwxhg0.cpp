/******************************************************************************

   This tutorial presents how to make a simple Web application.

   The difference between Web and a normal application,
      is that initially it doesn't have direct access to engine and project files.
   We need to download those files before the engine starts.
   
   Because of that, for Web platform you need to handle a new function called 'Preload'.
   Inside it you should download at least "Engine.pak".
   
   "Project.pak" can be downloaded at a later time, however for simplicity,
      we download it at the start as well.

   For this tutorial you need to:
      -set "Web JS" target platform
      -"Publish" the application
      -upload all the generated files to your website
      -start it from the website (Web Apps started from your local computer, may not work)

/******************************************************************************/
ImagePtr logo;
Download EnginePak, ProjectPak;
bool     finished;
/******************************************************************************/
void SetStatus(C Str &str=S) // execute JavaScript command to set text value of a HTML element
{
   JavaScriptRun(S+"Module.setStatus(\""+CString(str)+"\")");
}
/******************************************************************************/
bool Preload() // this will be called before 'InitPre', in here we'll download engine and project files
{
   if(finished)return false; // returning false means that we're done, and the engine can continue to 'InitPre' (in this tutorial we're actually returning one frame after we're done, so that the 'SetStatus' is able to finish setting the text)
   if(EnginePak.state()==DWNL_DONE && ProjectPak.state()==DWNL_DONE) // both files finished downloading
   {
      SetStatus("Initializing"); // set confirmation that now we'll be initializing data
      finished=true; // set that we're finished, but don't return false yet, do this next frame, so 'SetStatus' is able to update the text
   }else
   {
      if(! EnginePak.state()) EnginePak.create(EE_ENGINE_PATH ); // initialize engine  data download
      if(!ProjectPak.state())ProjectPak.create(EE_PROJECT_PATH); // initialize project data download
      
      int done=EnginePak.done()+ProjectPak.done(),
          size=EnginePak.size()+ProjectPak.size();
      SetStatus(S+"Downloading "+(size ? done*100/size : 0)+'%'); // set download progress text
   }
   return true;
}
void InitPre()
{
   EE_INIT(false, false); // the engine and project data files aren't included by default, so we can't load them, set false to specify that we're not loading them from files
   Paks.addMem( EnginePak.data(),  EnginePak.done()                   ); // load data from downloaded file
   Paks.addMem(ProjectPak.data(), ProjectPak.done(), EE_PROJECT_CIPHER); // load data from downloaded file

   D.mode(800*D.browserZoom(), 600*D.browserZoom()); // set custom resolution (this is optional, as by default resolution will be taken from the HTML canvas element)
   App.flag=APP_AUTO_FREE_OPEN_GL_ES_DATA|APP_AUTO_FREE_PHYS_BODY_HELPER_DATA|APP_CALLSTACK_ON_ERROR;
}
/******************************************************************************/
bool Init()
{
   logo=UID(1119600675, 1212460399, 80010661, 526665178);
   SetStatus(); // clear the status, as we're done initializing, and no longer need to display anything
   return true;
}
void Shut()
{
}
/******************************************************************************/
bool Update()
{
#if !WEB
   if(Kb.bp(KB_ESC))return false;
#endif
   return true;
}
void Draw()
{
   D.clear(WHITE);
   if(logo)logo->drawFs();
   REPA(MT)Circle(0.1, MT.pos(i)).draw(MT.b(i) ? RED : GREEN);
}
/******************************************************************************/
