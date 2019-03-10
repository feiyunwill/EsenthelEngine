/******************************************************************************/
Image    image;
Download download;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   download.create("http://www.esenthel.com/download/logo.gfx"); // create downloader

   return true;
}
/******************************************************************************/
void Shut()
{
   download.del(); // delete downloader
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;

   if(download.state()==DWNL_DONE) // if all data downloaded successfully
   {
      // data can be accessed freely through Download methods
      // we'll use the downloaded data to load an image from it

      File f(download.data(), download.size()); // create a file from memory data
      image.load(f);                            // load an image from the file

      download.del(); // delete the downloader
   }
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);

   if(image.is())     // when image is downloaded and created
      image.drawFs(); // draw it

   switch(download.state())
   {
      case DWNL_CONNECTING: D.text(0, 0,   "Awaiting for connection.."                         ); break;
      case DWNL_SENDING   : D.text(0, 0,   "Sending Request.."                                 ); break;
      case DWNL_DOWNLOAD  : D.text(0, 0, S+"Downloading.. "+download.done()+'/'+download.size()); break;
      case DWNL_ERROR     : D.text(0, 0,   "Error encountered"                                 ); break;
   }
}
/******************************************************************************/
