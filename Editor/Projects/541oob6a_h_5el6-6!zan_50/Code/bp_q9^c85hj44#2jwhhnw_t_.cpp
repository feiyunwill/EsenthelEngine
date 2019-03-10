/******************************************************************************

   Files are universal and can be used to:
      -read/write from stdio files (typical files on you Hard Drive)
      -read files from Pak         (an archive of multiple files)
      -read/write in   RAM         (without the need of writing files on Disk)

   They also handle encryption/decryption with custom Secure key.

/******************************************************************************/
byte b;
uint u;
Str  s;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   // for testing we'll start with with writing to a file in memory
   {
      File f; // file object

      f.writeMem(        ); // start writing to memory (file will automatically handle the memory management)
      f.putByte (     128); // put Byte      '128' to file
      f.putUInt (12345678); // put UInt '12345678' to file
      f.putStr  ("Test"  ); // put Str  "test"     to file
   }
   
   // now let's write all that but this time to a real file on Hard Disk
   {
      File f;

      f.write  ("file.dat"); // start writing to file
      f.putByte(     128);
      f.putUInt(12345678);
      f.putStr ("Test"  );
   }

   // when the file is created we can now read it
   {
      File f;
      
        f.read   ("file.dat"); // start reading file
      b=f.getByte(); // read Byte (returns 128)
      u=f.getUInt(); // read UInt (returns 12345678)
      s=f.getStr (); // read Str  (returns "Test")
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
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);
   D.text (0, 0, S+b+" "+u+" "+s); // display data obtained from file
}
/******************************************************************************/
