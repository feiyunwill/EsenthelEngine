/******************************************************************************/
const Str SettingsPath=S+EE_APP_NAME+".txt";
const flt Padd=0.015;
/******************************************************************************/
bool       LeftFocus, RecordTasks;
GuiSkin    TransparentSkin;
ImagePtr   FileIcon, FolderIcon, DriveIcon;
Pane        LeftPane;
Memx<Pane> RightPanes;
Tabs       RightTabs;
Button     ExploreLeft, ExploreRight, HostButton, NewRemoteButton, RenameButton, ReplaceButton, CreateFolderButton, DeleteFileButton;
ComboBox   Tasks;
Sound      sound;
MemcThreadSafe<Str> RefreshLocalPaths, TempNames;
/******************************************************************************/
Pane* ActiveRightPane () {return RightPanes.addr(RightTabs());}
Pane* ActiveRemotePane() {if(Pane *pane=ActiveRightPane())if(!pane.local && pane.connected())return pane; return null;}
Pane& ActivePane()
{
   if(!LeftFocus)if(Pane *pane=ActiveRightPane())return *pane;
   return LeftPane;
}
Pane* InactivePane()
{
   return LeftFocus ? ActiveRightPane() : &LeftPane;
}
Pane *FindPane(C UID &id)
{
   if(LeftPane.pane_id==id)return &LeftPane;
   REPA(RightPanes)if(RightPanes[i].pane_id==id)return &RightPanes[i];
   return null;
}
void RemovePaneDo(C UID &pane_id)
{
   if(Pane *pane=FindPane(pane_id))
      if(!pane.local)
   {
      int index=RightPanes.validIndex(pane);
      if(InRange(index, RightTabs))
      {
         RightPanes.removeValid(index, true);
         RightTabs .remove(index);
         RightTabs .set(Min(index, RightTabs.tabs()-1));
         Resize();
      }
   }
}
/******************************************************************************/
void Resize(flt old_w=0, flt old_h=0)
{
   const flt scale=1;
   D.scale(D.screenH()/flt(D.resH())*(950./1080));

   const flt h=0.06, w=h*4;
   RightTabs.rect(Rect_LU(Padd, D.h()-Padd, w*RightTabs.tabs(), h));
   NewRemoteButton.rect(Rect_LU(RightTabs.rect().ru(), h, h));

   RenameButton.rect(Rect_LD(-D.w()+Padd, -D.h()+Padd, 0.30, h));
   ReplaceButton.rect(Rect_LU(RenameButton.rect().ru()+Vec2(Padd, 0), 0.30, h));
   CreateFolderButton.rect(Rect_LU(ReplaceButton.rect().ru()+Vec2(Padd, 0), 0.4, h));
   DeleteFileButton.rect(Rect_LU(CreateFolderButton.rect().ru()+Vec2(Padd, 0), 0.28, h));
   Tasks.rect(Rect_LU(DeleteFileButton.rect().ru()+Vec2(h+Padd, 0), 0.18, h));

   LeftPane.resize(true);
   REPAO(RightPanes).resize(false);
   ExploreLeft.rect(Rect_RD(LeftPane.path.rect().ru(), 0.29, 0.06));
   if(RightPanes.elms())ExploreRight.rect(Rect_RD(RightPanes[0].path.rect().ru(), 0.29, 0.06));
   HostButton.rect(Rect_LD(LeftPane.up.rect().lu(), 0.3, 0.06));
}
void Resumed()
{
   LeftPane.refresh();
   REPA(RightPanes)if(RightPanes[i].local)RightPanes[i].refresh();
}
/******************************************************************************/
void LoadSettings()
{
   TextData d; if(d.load(SettingsPath))
   {
      if(TextNode *n=d.findNode("HostPath"))Host.path.set(n.asText());
      if(TextNode *n=d.findNode("Left" ))                                             LeftPane    .load(*n);
      if(TextNode *n=d.findNode("Right"))if(RightPanes.elms() && RightPanes[0].local)RightPanes[0].load(*n);
      for(int i=0; TextNode *r=d.findNode("Remote", i); i++)
      {
         Str name; if(TextNode *n=r.findNode("Name"))name=n.asText();
         NewRemoteDo(name).load(*r);
      }
      if(TextNode *n=d.findNode("RightActivePane"))RightTabs.set(n.asInt());
      for(int i=0; TextNode *t=d.findNode("Task", i); i++){TaskName tn; if(tn.load(*t))Swap(tn, TaskNames.New());}
   }
}
void SaveSettings()
{
   TextData d;
   d.nodes.New().set("HostPath"       , Host.path());
   d.nodes.New().set("RightActivePane", RightTabs());
   LeftPane.save(d.getNode("Left"));
   FREPA(RightTabs)
   {
      Pane     &pane=RightPanes[i];
      TextNode &node=d.nodes.New(); node.set(pane.local ? "Right" : "Remote");
      pane.save(node);
      if(!pane.local)node.nodes.New().set("Name", RightTabs.tab(i).text());
   }
   FREPA(TaskNames)
   {
      TextNode &t=d.nodes.New(); t.setName("Task");
      TaskNames[i].save(t);
   }
   d.save(SettingsPath);
}
/******************************************************************************/
Pane& NewRemoteDo(C Str &name)
{
   Pane &pane=RightPanes.New(); 
   pane.create(RightTabs.New(name));
   if(RightTabs()<0)RightTabs.set(0);
   Resize();
   return pane;
}
void HostDo(ptr) {if(HostButton())Host.activate();else HideWindow(Host);}
void ExploreLeftDo(ptr) {Explore(LeftPane.path());}
void ExploreRightDo(ptr) {if(RightPanes.elms() && RightPanes[0].local)Explore(RightPanes[0].path());}
void NewRemote(ptr) {NewRemoteDo("Remote");}
void RenameDo(ptr) {RenameFile.display();}
void ReplaceDo(ptr) {ReplaceFile.display();}
void CreateFolderDo(ptr) {CreateFolder.display();}
void DeleteDo(ptr) {DeleteFile.display();}
void MenuRenamePane() {RenamePane.display(PaneMenuID);}
void MenuRemovePane() {RemovePane.display(PaneMenuID);}
void MenuDisconnectPane() {if(Pane *pane=FindPane(PaneMenuID))pane.connectClear();}
void StopAllTasks() {LeftPane.stopTasks(); REPAO(RightPanes).stopTasks();}
void RecordTasksToggle()
{
   if(RecordTasks^=1)RecordedTasks.tasks.clear();else // start new recording
   if(!RecordedTasks.tasks.elms())Gui.msgBox(S, "No tasks recorded");else // save existing
      SaveTask.display(RecordedTasks);
}
void OrganizeTasks() {TaskOrganizer.activate();}
void ExecuteTasks(ptr p) {if(TaskName *tn=TaskNames.addr(intptr(p)))ExecuteTask.display(*tn);}
/******************************************************************************/
void InitPre()
{
   EE_INIT(false, false);
   App.flag=APP_RESIZABLE|APP_MINIMIZABLE|APP_MAXIMIZABLE|APP_FULL_TOGGLE|APP_NO_PAUSE_ON_WINDOW_MOVE_SIZE|APP_WORK_IN_BACKGROUND;
   App.resumed=Resumed;
   D.mode(App.desktopW()*0.8, App.desktopH()*0.8).shadowMapSize(0);
#if DEBUG
   Paks.add(EE_ENGINE_PATH);
   App.flag|=APP_BREAKPOINT_ON_ERROR|APP_CALLSTACK_ON_ERROR;
#else
   if(!EE_ENGINE_EMBED)Paks.add("Engine.pak");
#endif
   D.screen_changed=Resize;
   Gui.window_fade_in_speed=18;
   Gui.window_fade_out_speed=9;
}
bool Init()
{
   Node<MenuElm> menu;
   menu.New().create("Rename"    , MenuRenamePane);
   menu.New().create("Disconnect", MenuDisconnectPane);
   menu.New().create("Remove"    , MenuRemovePane);
   Gui+=PaneMenu.create();
   PaneMenu.list.elmHeight(0.05).textSize(0, 1);
   PaneMenu.setData(menu);

   FileIcon=UID(1122639237, 1220400833, 3502009774, 1102500093);
   FolderIcon=UID(3366946979, 1318930888, 3842680726, 2883731093);
   DriveIcon=UID(3055503367, 1267747133, 1268504206, 516465511);
   cchar8 *right[]=
   {
      "Local",
   };
   Gui.skin->keyboard_highlight_color=RED;
   TransparentSkin=*Gui.skin;
   TransparentSkin.region. normal_color.a/=2;
   TransparentSkin.list.   cursor_color.a/=2;
   TransparentSkin.list.selection_color.a/=2;
   TransparentSkin.list.highlight_color.a/=2;
   LeftPane.create(*Gui.desktop(), true);
   Gui+=RightTabs.create(right, Elms(right), false).valid(true).set(0);
   RightPanes.New().create(RightTabs.tab(0), true);
   RightTabs.tab(0)+=ExploreRight.create("Explore (F5)").func(ExploreRightDo).focusable(false).desc("Open path in System Explorer");
   Gui+=NewRemoteButton.create("+").func(NewRemote).focusable(false).desc("Create a new Pane");
   Gui+=ExploreLeft.create("Explore (F4)").func(ExploreLeftDo).focusable(false).desc("Open path in System Explorer");
   Gui+=HostButton.create(S+"Host ("+Kb.ctrlCmdName()+"+H)").func(HostDo).focusable(false); HostButton.mode=BUTTON_TOGGLE;
   Gui+=RenameButton.create("Rename (F2)").func(RenameDo).focusable(false);
   Gui+=ReplaceButton.create("Replace (F6)").func(ReplaceDo).focusable(false);
   Gui+=CreateFolderButton.create("Create Folder (F7)").func(CreateFolderDo).focusable(false);
   Gui+=DeleteFileButton.create("Delete (Del)").func(DeleteDo).focusable(false);
   Gui+=Tasks.create().focusable(false); Tasks.text="Tasks"; Tasks.flag=COMBOBOX_CONST_TEXT;
   RenamePane.create();
   RenameFile.create();
   RenameTask.create();
   ReplaceFile.create();
   CreateFolder.create();
   DeleteFile.create();
   DeleteTask.create();
   RemovePane.create();
   SaveTask.create();
   ExecuteTask.create();
   TaskOrganizer.create();
   Host.create();
   Resize();
   LoadSettings();
   LeftPane.refreshJumpList();
   REPAO(RightPanes).refreshJumpList();
   RefreshTasks();
   LeftPane.list.kbSet();
   StartCompress();
   StartIO();
   return true;
}
void Shut()
{
   SaveSettings();
   IOThread      .del();
   CompressThread.del();
   Server        .del();
   RightPanes    .del();
   RightTabs     .del();
   TempNames.lock  (); REPA(TempNames)FDel(TempNames.lockedElm(i));
   TempNames.unlock();
}
/******************************************************************************/
void SetRightFocus()
{
   if(Pane *pane=ActiveRightPane())pane.list.kbSet();
}
void SetFocus()
{
   if(LeftFocus)LeftPane.list.kbSet();
   else         SetRightFocus();
}
void GetFocus()
{
   if(Gui.kb()==&LeftPane.list)LeftFocus=true;else
   if(RightTabs.contains(Gui.kb()))LeftFocus=false;
}
bool Update()
{
   if(!App.active())Time.wait(1);
#if DEBUG
   if(Kb.bp(KB_ESC) && !Gui.window())return false;
#endif
   if(Kb.k(KB_TAB) && !Gui.window()) // process TAB before Gui.update
   {
      if(Kb.k.ctrlCmd() && RightTabs.tabs())RightTabs.set(Mod(RightTabs()-SignBool(Kb.k.shift()), RightTabs.tabs()));else
      if(Gui.kb()==&LeftPane.list)SetRightFocus();else
      if(RightTabs.contains(Gui.kb()))LeftPane.list.kbSet();else
         SetFocus();
      Kb.eatKey();
   }
   if(Kb.ctrlCmd())
   {
      if(Kb.b(KB_1))RightTabs.set(0);
      if(Kb.b(KB_2))RightTabs.set(1);
      if(Kb.b(KB_3))RightTabs.set(2);
      if(Kb.b(KB_4))RightTabs.set(3);
      if(Kb.b(KB_5))RightTabs.set(4);
      if(Kb.b(KB_6))RightTabs.set(5);
      if(Kb.b(KB_7))RightTabs.set(6);
      if(Kb.b(KB_8))RightTabs.set(7);
      if(Kb.b(KB_9))RightTabs.set(8);
      if(Kb.b(KB_0))RightTabs.set(9);
   }
   GetFocus();
   Gui.update();
   Server.update();

   if(KbSc(KB_P, KBSC_CTRL_CMD).pd()){Pane &pane=ActivePane(); if(Gui.kb()==&pane.path)SetFocus();else pane.path.selectAll().kbSet();}
   if(Kb.bp(KB_ESC))SetFocus();
   if(Kb.k(KB_F1))if(Pane *pane=&ActivePane())if(pane.local)if(Pane.Elm *elm=pane.list())
   {
      Str full=pane.getPath()+elm.name;
      sound.play(full);
      pane.list.setCur(Mod(pane.list.cur-SignBool(Kb.k.shift()), pane.list.visibleElms()));
      pane.list.scrollTo(pane.list.cur, false, 0.5);
   }
   if(Kb.bp(KB_F2))RenameButton.push();
   if(Kb.bp(KB_F4))ExploreLeft.push();
   if(Kb.bp(KB_F5))ExploreRight.push();
   if(Kb.bp(KB_F6))ReplaceButton.push();
   if(Kb.bp(KB_F7))CreateFolderButton.push();
   if(Kb.kf(KB_DEL) || Kb.kf(KB_NPDEL))DeleteFileButton.push();
   if(KbSc(KB_H, KBSC_CTRL_CMD).pd())HostButton.push();

   if(RefreshLocalPaths.elms())
   {
      RefreshLocalPaths.  lock(); Mems<Str> refresh; refresh.setNum(RefreshLocalPaths.elms()); REPA(RefreshLocalPaths)refresh[i]=RefreshLocalPaths.lockedElm(i); RefreshLocalPaths.clear();
      RefreshLocalPaths.unlock();
             LeftPane  .refreshLocalPath(refresh);
      REPAO(RightPanes).refreshLocalPath(refresh);
   }

          LeftPane  .update(true );
   REPAO(RightPanes).update(false);

   // pane menu
   if(Ms.bp(1))REPA(RightTabs)if(Gui.ms()==&RightTabs.tab(i) && !RightPanes[i].local)
   {
      PaneMenuID=RightPanes[i].pane_id;
      PaneMenu.activate().posRU(Ms.pos());
   }

   // close pane
   if(Ms.bp(2))REPA(RightTabs)if(Gui.ms()==&RightTabs.tab(i) && !RightPanes[i].local){RemovePaneDo(RightPanes[i].pane_id); break;}

   return true;
}
/******************************************************************************/
void Draw()
{
   D.clear(TURQ);
   Gui.draw();
}
/******************************************************************************/
