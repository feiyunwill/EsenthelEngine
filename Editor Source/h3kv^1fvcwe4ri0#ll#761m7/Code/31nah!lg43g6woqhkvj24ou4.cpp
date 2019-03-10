/******************************************************************************/
Memx<ProjectHierarchy> Projects;
Str                    ProjectsPath;
/******************************************************************************/
bool RenameProject(C UID &id, C Str &name)
{
   if(Project *proj=FindProject(id))
   {
      Str clean=CleanFileName(name);
      if(!ValidFileName(clean))
      {
         Gui.msgBox(S, "Project name can't be empty, can't start with dot and can't contain following symbols \\ / : * ? \" < > |");
         return false;
      }
      if(!Equal(proj.name, clean, true)) // setting new name
      {
         proj.name=clean;
         Server.distributeProjList(); // send new list to all users
      }
   }
   return true;
}
/******************************************************************************/
ProjectHierarchy* FindProject(C UID &id)
{
   REPA(Projects)if(Projects[i].id==id)return &Projects[i];
   return null;
}
/******************************************************************************/
ProjectHierarchy* GetProject(C UID &id, C Str &name)
{
   if(id.valid())
   {
      if(ProjectHierarchy *proj=FindProject(id))return proj; // find existing from memory
      // load from disk
      ProjectHierarchy temp; Str error; if(LoadOK(temp.open(id, name, ProjectsPath+EncodeFileName(id), error))) // try to load
      {
         ProjectHierarchy &proj=Projects.New(); Swap(temp, proj);
         proj.save(); // save because it didn't exist yet
         PV.refresh();
         Server.distributeProjList(); // send new list to all users
         return &proj;
      }
      Gui.msgBox(S, S+"Can't create project \""+name+'"');
   }
   return null;
}
/******************************************************************************/
void LoadProjects()
{
   ProjectHierarchy temp; for(FileFind ff(ProjectsPath); ff(); )if(temp.isProject(ff))
   {
      Str error;
      if(LoadOK(temp.open(temp.id, temp.name, ff.pathName(), error, true))) // ignore lock because currently there's no way to unlock manually, TODO: add some gui for this
         Swap(Projects.New(), temp);
   }
}
/******************************************************************************/
