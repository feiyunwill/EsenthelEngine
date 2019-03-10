/******************************************************************************/
WindowIO wio; // window handling input/ouput
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
void Load(C Str &name, ptr user) // this function will be called when 'WindowIO' will load a file
{
}
void Save(C Str &name, ptr user) // this function will be called when 'WindowIO' will save a file
{
}
/******************************************************************************/
bool Init()
{
   wio.create("ext", S, S, Load, Save); // create a 'WindowIO', accepting given file extension, paths, and IO functions
   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC) && !Gui.window())return false;
   Gui.update();

   // activate saving/loading on keypress
   if(Kb.bp(KB_F2))wio.save();
   if(Kb.bp(KB_F3))wio.load();

   return true;
}
/******************************************************************************/
void Draw()
{
   D  .clear(TURQ);
   D  .text(0, 0.9, "Press F2/F3 to save/load");
   Gui.draw();
}
/******************************************************************************/
