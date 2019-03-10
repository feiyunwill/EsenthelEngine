/******************************************************************************/
bool    Initialized=false;
flt     SaveTime=AutoSaveTime;
Threads WorkerThreads;
/******************************************************************************/
void ScreenChanged(flt old_width=0, flt old_height=0)
{
   flt scale=D.screenH()/1080.0;
   D.scale(scale*600./D.resH());
   ResizeGui();
}
/******************************************************************************/
void InitPre()
{
   ASSERT(CS_NUM<=256); // because they are stored as bytes
   IsServer=true;

   ProjectsPath=GetPath(App.exe())+"\\Projects (Server)\\";
   App.flag=APP_MINIMIZABLE|APP_MAXIMIZABLE|APP_NO_PAUSE_ON_WINDOW_MOVE_SIZE|APP_WORK_IN_BACKGROUND|APP_RESIZABLE|APP_FULL_TOGGLE|APP_ALLOW_NO_XDISPLAY;
#if WINDOWS
   App.flag|=APP_ALLOW_NO_GPU;
#endif
   EE_INIT(false, false);
   App.name(ServerAppName);
   if(!EE_ENGINE_EMBED)
   {
   #if DEBUG
      Paks.add(EE_ENGINE_PATH);
   #else
      Paks.add("Bin/Engine.pak");
   #endif
   }
   Renderer.type(RT_SIMPLE);
   D.screen_changed=ScreenChanged;
   D.shadowMode(SHADOW_NONE).shadowMapSize(0);
   flt scale=D.screenH()/1080.0;
   D.mode(650*scale, 320*scale);
   WorkerThreads.create(true);
}
/******************************************************************************/
bool Init()
{
   if(!Server.create())Exit("Can't create Server"); // create server
   InitGui();

   Materials  .mode(CACHE_ALL_DUMMY); // because Materials are loaded for syncing ('ServerRecvSetWorldArea')
   Images     .mode(CACHE_ALL_DUMMY); // because Materials are loaded for syncing ('ElmMaterial.sync')
   EditObjects.mode(CACHE_ALL_DUMMY); // because ...
       Objects.mode(CACHE_ALL_DUMMY); // because Object uses this
   Enums      .mode(CACHE_ALL_DUMMY); // because Object uses this

   ScreenChanged();
   LoadSettings();
   LoadProjects();
   PV.refresh();
   UV.refresh();
   InitMailer();
   Initialized=true;
   REPA(Projects)if(Projects[i].needUpdate()){StateProjectUpdate.set(); break;}
   return true;
}
void Shut()
{
   WorkerThreads.del();
   ShutMailer();
   Server.del();
   REPAO(Projects).close();
   if(Initialized)SaveSettings();
}
/******************************************************************************/
bool Update()
{
   CurTime.getUTC();
   Server.update(); // update server, this accepts new clients and processes all existing
   Gui.update();
   Time.wait(1);

   // auto save
   if(Time.appTime()>=SaveTime)
   {
      SaveTime=Time.appTime()+AutoSaveTime;
      REPAO(Projects).save();
   }
   return true;
}
void Draw()
{
   D.clear(WHITE);
   if(Mode()==MODE_CONNECTION)
   {
      TextStyleParams ts(true); ts.align.set(1, -1); ts.size=TextHeight; flt y=Mode.rect().min.y-0.01, h=ts.size.y;

      // draw debug info
      D.text(ts, -D.w(), y, S+"Local Address: " +Server.localAddress().asText()); y-=h;
      D.text(ts, -D.w(), y, S+"Global Address: "+(bGlobalAddressResolve.hidden() ? Server.globalAddress().asText() : "")); y-=h;
      D.text(ts, -D.w(), y, S+"Uptime: "+TimeText(Round(Time.realTime()))+", Clients: "+Server.clients.elms()+" (peak: "+Server.clients_peak+")"); y-=h;

      // traffic
      long sent    =Server.bytesSentTotal    (),
           received=Server.bytesReceivedTotal();
      D.text(ts, -D.w(), y, S+"Received: "+TextInt(received>>10, -1, 3)+" KB, Sent: "+TextInt(sent>>10, -1, 3)+" KB"); y-=h;
      
      // about
      D.text(ts, -D.w(), y, S+"Network Protocol: "+ClientServerVersion); y-=h;
   }
   Gui.draw();
}
/******************************************************************************/
