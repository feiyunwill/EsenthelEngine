/******************************************************************************/
void InitPre() // initialize before engine inits
{
   EE_INIT(); // call auto-generated function that will setup application name, load engine and project data
}
/******************************************************************************/
bool Init() // initialize after engine is ready
{
   // here when engine will be ready you can load your game data
   // return false when error occurred
   return true;
}
/******************************************************************************/
void Shut() // shut down at exit
{
   // this is called when the engine is about to exit
}
/******************************************************************************/
bool Update() // main updating
{
   // here you have to process each frame update

   if(Kb.bp(KB_ESC))return false; // exit if escape on the keyboard pressed
   return true;                   // continue
}
/******************************************************************************/
void Draw() // main drawing
{
   // here you have to tell engine what to draw on the screen

   D.clear(TURQ); // clear screen to turquoise color
   D.text (0, 0, "Hello to Esenthel Engine !"); // display text at (0, 0) screen coordinates
}
/******************************************************************************/
