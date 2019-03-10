/******************************************************************************/
Mems<ProjectUpdate> ProjUpdate;
State               StateProjectUpdate(UpdateProjectUpdate, DrawProjectUpdate, InitProjectUpdate, ShutProjectUpdate);
/******************************************************************************/
bool InitProjectUpdate()
{
   UpdateProgress.create(Rect_C(0, -0.05, 1, 0.045));
   ProjUpdate.setNum(Projects.elms()); FREPAO(ProjUpdate).start(Projects[i], WorkerThreads); // start at the end
   return true;
}
void ShutProjectUpdate()
{
   REPAO(ProjUpdate).stop(WorkerThreads); ProjUpdate.del(); // stop first
   UpdateProgress.del();
   WindowSetNormal(); WindowFlash();
}
/******************************************************************************/
bool UpdateProjectUpdate()
{
   if(Kb.bp(KB_ESC))return false; // can't go to the main state because didn't finish updating, so close app
   if(!WorkerThreads.busy())
   {
      REPA(Projects)if(Projects[i].needUpdate()) // if after updating, the project still needs an update, then it means that some error must have occured
      {
         WindowMsgBox("Error", "Projects did not update completely"); // display OS message box
         return false; // and exit
      }
      StateMain.set(StateFadeTime);
   }

   int total=0; REPA(ProjUpdate)total+=ProjUpdate[i].texs.elms();
   UpdateProgress.set(total-WorkerThreads.queued(), total);
   WindowSetProgress(UpdateProgress());
   Time.wait(1000/30);
 //Gui.update(); this may cause conflicts with 'Proj.elmChanged'
   return true;
}
/******************************************************************************/
void DrawProjectUpdate()
{
   D.clear(BackgroundColor());
   D.text(0, 0.05, "Updating Projects");
   GuiPC gpc;
   gpc.visible=gpc.enabled=true; 
   gpc.client_rect=gpc.clip.set(-D.w(), -D.h(), D.w(), D.h());
   gpc.offset.zero();
   UpdateProgress.draw(gpc);
   D.clip();
}
/******************************************************************************/
