/******************************************************************************/
class HostClass : ClosableWindow
{
   Text      info, t_path, t_pass, local_addr, global_addr;
   TextLine  path, pass;
   Button    host, copy_local, copy_global;
   TextStyle ts;

   static void CopyLocal (HostClass &host) {ClipSet(Server.localAddress().asText());}
   static void CopyGlobal(HostClass &host)
   {
      if(host.copy_global.text=="Copy")ClipSet(Server.globalAddress().asText());
      {
         host.copy_global.text="Copy";
         host.global_addr.set(S+"Global Address: "+Server.globalAddress().asText());
      }
   }
   static void Host(HostClass &host)
   {
      if(host.host())
      {
         FileInfoSystem fi(host.path());
         if(host.path().is())if(fi.type!=FSTD_DIR && fi.type!=FSTD_DRIVE || !FullPath(host.path())){Gui.msgBox(S, S+"\""+host.path()+"\"\nIs not a valid path."); host.host.set(false, QUIET);}
      }
      if(host.host())
      {
         Server.create();
         host.local_addr.set(S+"Local Address: "+Server.localAddress().asText());
         host.copy_global.text="Resolve";
      }else
      {
         Server.del();
         host.global_addr.set("Global Address:");
      }
      host. local_addr.visible(host.host());
      host.global_addr.visible(host.host());
      host. copy_local.visible(host.host());
      host.copy_global.visible(host.host());
      host.path.disabled(host.host());
      host.pass.disabled(host.host());
      host.pass.kbSet();
   }

   void create()
   {
      ts.reset(true); ts.align.set(1, 0);
      Gui+=super      .create(Rect_C(0, 0, 1.3, 0.65), "Host File Server").hide(); button[2].func(HideWindow, SCAST(Window, T)).show();
      T  +=info       .create(Vec2(clientWidth()/2, -0.07), "Warning: Clients will have full access (read/write) to selected path!\nUse a safe folder with a secure password.");
      T  +=t_path     .create(Vec2(0.22, -0.18), "Path to Share"      ); T+=path.create(Rect_L(0.44, -0.18, 0.83, 0.057));
      T  +=t_pass     .create(Vec2(0.22, -0.25), "Connection Password"); T+=pass.create(Rect_L(0.44, -0.25, 0.83, 0.057), Random.password(6, true, true));
      T  +=host       .create(Rect_C(clientWidth()/2, -0.35, 0.3, 0.065), "Host").focusable(false).func(Host, T); host.mode=BUTTON_TOGGLE;
      T  += local_addr.create(Vec2(0.2, -0.44), S, &ts); T+=copy_local .create(Rect_L(0.92, -0.44, 0.18, 0.055), "Copy").func(CopyLocal, T).focusable(false);
      T  +=global_addr.create(Vec2(0.2, -0.51), S, &ts); T+=copy_global.create(Rect_L(0.92, -0.51, 0.18, 0.055)).func(CopyGlobal, T).focusable(false);
      Host(T);
      pass.selectAll().kbSet();
   }
   virtual HostClass& show() {super.show(); if(!host() && !path().is())path.set(LeftPane.path()); HostButton.set(true , QUIET); return T;}
   virtual HostClass& hide() {super.hide();                                                       HostButton.set(false, QUIET); return T;}

   virtual void update(C GuiPC &gpc)
   {
      super.update(gpc);

      if(Gui.window()==this)
      {
         if(Kb.k(KB_ENTER))
         {
            host.push();
            Kb.eatKey();
         }
      }
   }
}
HostClass Host;
/******************************************************************************/
