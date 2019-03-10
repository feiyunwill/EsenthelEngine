/******************************************************************************/
ProjectUpdate ProjUpdate;
State         StateProjectUpdate(UpdateProjectUpdate, DrawProjectUpdate, InitProjectUpdate, ShutProjectUpdate);
/******************************************************************************/
bool InitProjectUpdate()
{
   SetKbExclusive();
   Proj.pause(); // pause first
   UpdateProgress.create(Rect_C(0, -0.05, 1, 0.045));
   ProjUpdate.start(Proj, WorkerThreads); // start at the end
   return true;
}
void ShutProjectUpdate()
{
   ProjUpdate.stop(WorkerThreads); // stop first
   UpdateProgress.del();
   Proj.refresh().resume(); // resume at the end
   WindowSetNormal();
   WindowFlash();
}
/******************************************************************************/
bool UpdateProjectUpdate()
{
   if(Kb.bp(KB_ESC)){StateProjectList.set(StateFadeTime); Gui.msgBox(S, "Updating Project breaked on user request");}
   if(!WorkerThreads.busy())
   {
      if(Proj.needUpdate()) // if after updating, the project still needs an update, then it means that some error must have occured
      {
         Proj.close();
         Gui.msgBox(S, "Project did not update completely");
      }
      SetProjectState();
   }

   UpdateProgress.set(ProjUpdate.texs.elms()-WorkerThreads.queued(), ProjUpdate.texs.elms());
   WindowSetProgress(UpdateProgress());
   Time.wait(1000/30);
     //Gui.update(); this may cause conflicts with 'Proj.elmChanged'
    Server.update(null, true);
   if(Ms.bp(3))WindowToggle();
   return true;
}
/******************************************************************************/
void DrawProjectUpdate()
{
   D.clear(BackgroundColor());
   D.text(0, 0.05, "Updating Project");
   GuiPC gpc;
   gpc.visible=gpc.enabled=true; 
   gpc.client_rect=gpc.clip.set(-D.w(), -D.h(), D.w(), D.h());
   gpc.offset.zero();
   UpdateProgress.draw(gpc);
   D.clip();
}
/******************************************************************************/
