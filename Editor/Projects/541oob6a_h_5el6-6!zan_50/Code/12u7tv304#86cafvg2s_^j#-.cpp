/******************************************************************************

   Esenthel Engine allows you to programatically connect to the Editor,
      and operate on projects and their elements.

   This tutorial will present how to connect to the Editor, and obtain a list of projects.

/******************************************************************************/
Edit.EditorInterface EI; // Editor Interface, which handles connecting and exchanging data between the application and the editor
Memc<Edit.Project> Projects; // list of projects
UID CurProject=UIDZero; // current project
/******************************************************************************/
Edit.Project* FindProject(MemPtr<Edit.Project> projects, C UID &id)
{
   REPA(projects)if(projects[i].id==id)return &projects[i];
   return null;
}
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

   EI.getProjects(Projects); // get a list of projects in the Editor
   CurProject=EI.curProject(); // get current project

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
   Edit.Project *cur=FindProject(Projects, CurProject);
   D.text(0, 0.90, S+"Current Project: "+(cur ? cur.name : "<none>"));
   D.text(0, 0.75, "All Projects:");
   flt y=0.65;  FREPA(Projects){D.text(0, y, Projects[i].name); y-=0.07;}
}
/******************************************************************************/
