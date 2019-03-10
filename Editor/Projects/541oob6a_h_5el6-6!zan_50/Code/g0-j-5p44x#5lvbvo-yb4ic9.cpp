/******************************************************************************/
Memc<Str> names;
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
void AddName(C Str &name) // add 'name' to names list
{
   names.add(name); // create new element in names and set it's name
}
/******************************************************************************/
void ManualFind(C Str &path) // manually iterate through directories and files
{
   for(FileFind ff(path); ff();)switch(ff.type) // start looking for files in path, continue while active, and check for encountered type
   {
      case FSTD_DIR : ManualFind(ff.pathName()); break; // if directory encountered start looking inside it
      case FSTD_FILE: AddName   (ff.pathName()); break; // if file      encountered add it to the list
   }
}
/******************************************************************************/
FILE_LIST_MODE AutoFind(C FileFind &ff, Ptr user)
{
   if(ff.type==FSTD_FILE)AddName(ff.pathName());
   return FILE_LIST_CONTINUE;
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;

   if(Kb.bp(KB_SPACE))
   {
      names.clear();             // clear elements
      FList("source", AutoFind); // use 'FList' which operates on all files inside given path ('AutoFind' will be called multiple times with each file as the parameter)
   }

   if(Kb.bp(KB_ENTER))
   {
      names.clear();        // clear elements
      ManualFind("source"); // manually iterate through all directories and files
   }

   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);
   D.text (0, 0.9, "Press Space to use 'FList', or press Enter for 'FileFind'");

   FREPA(names)D.text(0, 0.8-i*0.1, names[i]); // draw all file names
}
/******************************************************************************/
