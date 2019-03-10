/******************************************************************************

   !! BEFORE RUNNING THIS TUTORIAL, PLEASE READ THE INSTRUCTIONS ON HOW TO SETUP FACEBOOK !!
   !! INSTRUCTIONS ARE IN "Esenthel Engine\Net\Facebook" Header File                      !!

/******************************************************************************/
const bool IHaveReadTheInstructionsAtTheTop=false; // read the instructions above, then set this to true

const bool SetPhotoSize=true; // if download photos based on targetted screen resolution

Button        DisplayFriends, PostOnWall, OpenPage;
InternetCache IC; // cache for images from the internet
Threads       Workers; // worker threads for importing images in the background thread
/******************************************************************************/
void GetFriends(ptr)
{
   FB.getFriends(); // download list of friends
}
void PostOnWallDo(ptr)
{
   FB.post(S, "http://www.esenthel.com/", S, "Esenthel Engine is Awesome", "Yes it's true");
}
void OpenPageDo(ptr)
{
   FB.openPage("EsenthelEngine", "161038147263508");
}
/******************************************************************************/
void Save()
{
   IC.flush(); // flush downloaded data to disk
}
void InitPre()
{
   ASSERT(IHaveReadTheInstructionsAtTheTop);
   EE_INIT();
   App.save_state=Save;
}
/******************************************************************************/
bool Init()
{
   Workers.create(true, Max(1, Cpu.threads()-1)); // create workers
   Str path;
#if MOBILE
   path=SystemPath(SP_APP_DATA);
#endif
   IC.create(path.tailSlash(true)+"InternetCache.dat", &Workers); // create internet cache

   Images.delayRemove(30); // this will delay automatic unloading of cached images

   Gui+=DisplayFriends.create(Rect_C(0,  0.2, 0.8, 0.12), "Display Friends").func(GetFriends);
   Gui+=PostOnWall    .create(Rect_C(0,  0.0, 0.8, 0.12), "Post On Wall"   ).func(PostOnWallDo);
   Gui+=OpenPage      .create(Rect_C(0, -0.2, 0.8, 0.12), "Open Page"      ).func(OpenPageDo);

   return true;
}
/******************************************************************************/
void Shut()
{
   Save();
}
/******************************************************************************/
bool Update()
{
   Gui.update();
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(WHITE);

   // display friends
   flt h=0.15;
   TextStyleParams ts; ts.align.set(1, 0);
   FREPA(FB.friends())
   {
    C Facebook.User &f=FB.friends()[i];
      Rect_LU rect(-D.w(), D.h()-i*h, h, h);
      int     size_px=-1; if(SetPhotoSize)size_px=Round(D.screenToPixelSize(rect.size()).max());

      if(i<10) // display photos only for first 10 people
      {
         // WARNING: normally 'photo' would be loaded and unloaded everytime because of reference counting
         // this would make the following code VERY inefficient
         // however in this tutorial we have called 'Images.delayRemove' which delays auto unloading
         if(ImagePtr photo=IC.getImage(FB.userImageURL(f.id, size_px, size_px)))
            photo->drawFit(rect);
      }

      D.text(ts, rect.right(), S+"id:"+f.id+", name:"+f.name);
   }

   Gui.draw();
}
/******************************************************************************/
