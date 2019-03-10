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
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(WHITE);

   // Each resource in the project has its unique ID (UID).
   // You can check the ID of an element by right clicking on it, and selecting "Properties".
   // From there you can copy its "ID", or its "File ID" so it can be used in the codes.
   // To do this faster, you can use following methods:
   // -Drag and drop                 an element on the source code which will copy its      ID into mouse   position in the codes.
   // -Ctrl +         Right click on an element                    which will copy its      ID into current position in the codes.
   // -Ctrl + Shift + Right click on an element                    which will copy its File ID into current position in the codes.
   // -Ctrl + Shift + Space       to display list of project elements and select the one that you're interested in.

   // Let's draw an image from project resources, we need to use 'Images' cache,
   // to access an Image through its ID this way:
   Images(UID(1119600675, 1212460399, 80010661, 526665178))->drawFs(); // draw image full screen

   // If you'd like to investigate the project element by its ID, simply:
   // Copy the whole "UID(..)" text into "Find element" textline in the project view,
   // and desired element will get highlighted.
}
/******************************************************************************/
