/******************************************************************************

   This tutorial will present how to preview an element from the project.

/******************************************************************************/
Edit.EditorInterface EI;
ImagePtr image;
Str      image_name;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
}
/******************************************************************************/
bool Init()
{
   // connect to a running instance of the Editor
   Str message; if(!EI.connect(message))Exit(message);

   Memc<Edit.Elm> elms; EI.getElms(elms); // get a list of project elements
   Str data_path=EI.dataPath(); // get game data path folder (which contains all game data files)

   FREPA(elms)if(elms[i].type==Edit.ELM_IMAGE) // look for a first image in the project
   {
      Str elm_file=Edit.EditorInterface.ElmFileName(elms[i].id, data_path); // get the element file
      image     =elm_file; // load image from that file
      image_name=elms[i].full_name;
      break; // stop
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
   D.clear(BLACK);
   if(image)image->drawFs();
   D.text(0, 0, S+"Image: \""+image_name+'"');
}
/******************************************************************************/
