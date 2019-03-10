/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
bool Init()
{
   Ftp ftp; // create and Ftp object

   // login to the FTP
   if(ftp.login("ftp.host.com", "user", "password")) // in order to connect to an FTP server you will need to change these parameters to your custom
   {
      File file;

      // sample upload
      file.read  ("c:/test.file");     // open file for reading
      ftp .upload(file, "/test.file"); // upload "c:/test.file" to the FTP "/test.file"

      // sample download
      file.writeMem();                   // open file for writing to memory
      ftp .download("/test.file", file); // download "/test.file" from the FTP to 'file'

      // sample list files
      Memc<FtpFile> files;       // container for the files
      ftp.listFiles("/", files); // list all files in the root "/" ftp to the 'files' container

      // logout
      ftp.logout();
   }

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
   return true;
}
void Draw()
{
   D.clear(WHITE);
}
/******************************************************************************/
