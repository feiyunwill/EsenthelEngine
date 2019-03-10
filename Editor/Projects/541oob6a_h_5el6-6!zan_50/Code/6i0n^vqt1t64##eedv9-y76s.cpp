/******************************************************************************/
Memc<Str> FileNames;
/******************************************************************************/
void Drop(Memc<Str> &names, GuiObj *obj, C Vec2 &screen_pos)
{
   FileNames=names; // copy 'names' to global variable 'FileNames'
}
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   App.drop=Drop; // enable Drag & Drop by specifying a custom function which will be called when a file is dropped on the window
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
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);
   D.text (0, 0.9, "Drag & Drop some files to this window");

   FREPA(FileNames)D.text(0, 0.7-i*0.1, FileNames[i]); // draw all file names
}
/******************************************************************************/
