/******************************************************************************/
class Str2
{
   Str src, dest;

   void set(C Str &src, C Str &dest=S) {T.src=src; T.dest=dest;}

   bool save(File &f)C {f<<src<<dest; return f.ok();}
   bool load(File &f)  {f>>src>>dest; return f.ok();}
}
/******************************************************************************/
class Task
{
   COMMAND    type=CMD_NUM; // set invalid at start
   bool       src_local=true, dest_local=true;
   Memc<Str2> names;

   cchar8* commandName()C {return CommandName(type);}

   bool usesDestPath ()C {return UsesDestPath (type);}
   bool usesDestNames()C {return UsesDestNames(type);}

   bool usesRemote()C {return !src_local || !dest_local && usesDestPath();}

   bool saveSrcNames(File &f)C {  f.cmpUIntV(names.elms()); FREPA(names)f<<names[i].src; return f.ok();}
   bool loadSrcNames(File &f)  {names.setNum(f.decUIntV()); FREPA(names)f>>names[i].src; return f.ok();}

   bool saveNames(File &f)C {return usesDestNames() ? names.save(f) : saveSrcNames(f);}
   bool loadNames(File &f)  {return usesDestNames() ? names.load(f) : loadSrcNames(f);}

   void reset() {T=Task();}

   void save(TextNode &node)C
   {
      node.name=commandName();
      if(usesDestPath())
      {
         node.nodes.New().set("Src" ,  src_local ? "Local" : "Remote");
         node.nodes.New().set("Dest", dest_local ? "Local" : "Remote");
      }else
      {
         node.nodes.New().set("Path", src_local ? "Local" : "Remote");
      }
      TextNode &elms=node.getNode("Elms");
      bool uses_dest_names=usesDestNames();
      FREPA(names)
      {
       C Str2 &name=names[i];
         TextNode &file=elms.nodes.New();
         if(uses_dest_names)
         {
            file.getNode("Src" ).value=name.src;
            file.getNode("Dest").value=name.dest;
         }else
         {
            file.value=name.src;
         }
      }
   }
   bool load(TextNode &node)
   {
      if(node.name.is())REP(CMD_NUM)if(node.name==CommandName(COMMAND(i)))
      {
         reset();
         type=COMMAND(i); // set at start
         if(usesDestPath())
         {
            if(TextNode *src =node.findNode("Src" ))if(src .value=="Local") src_local=true;else if(src .value=="Remote") src_local=false; // else keep default
            if(TextNode *dest=node.findNode("Dest"))if(dest.value=="Local")dest_local=true;else if(dest.value=="Remote")dest_local=false; // else keep default
         }else
         {
            if(TextNode *path=node.findNode("Path"))if(path.value=="Local")src_local=true;else if(path.value=="Remote")src_local=false; // else keep default
         }
         if(TextNode *elms=node.findNode("Elms"))
         {
            bool uses_dest_names=usesDestNames();
            FREPA(elms.nodes)
            {
               TextNode &file=elms.nodes[i];
               if(uses_dest_names)
               {
                  if(TextNode *src =file.findNode("Src" ))if(src .value.is())
                  if(TextNode *dest=file.findNode("Dest"))if(dest.value.is())names.New().set(src.value, dest.value);
               }else
               {
                  if(file.value.is())names.New().set(file.value);
               }
            }
            return names.elms()>0;
         }
      }
      return false;
   }
   void save(MemPtr<Str> lines)C
   {
      FREPA(names)
      {
       C Str2 &name=names[i];
         Str  &line=lines.New();
         line+=commandName();
                             line+=S+   " \""+GetBase(name.src )+"\" in \""+GetPath(name.src )+'"'; if(! src_local)line+=" (REMOTE)";
         if(usesDestNames()){line+=S+" to \""+GetBase(name.dest)+"\" in \""+GetPath(name.dest)+'"'; if(!dest_local)line+=" (REMOTE)";}
      }
   }

   Task(COMMAND type, bool src_local, bool dest_local=true) : type(type), src_local(src_local), dest_local(dest_local) {}
}
/******************************************************************************/
class TaskName
{
   Str        name;
   Memc<Task> tasks;
   UID        id;

   TaskName() {id.randomize();}

   bool usesRemote()C
   {
      REPA(tasks)if(tasks[i].usesRemote())return true;
      return false;
   }
   void execute(Pane &pane)C
   {
      FREPA(tasks)pane.threadTask(tasks[i]); // process in order
   }
   void save(TextNode &node)C
   {
      node.getNode("Name").value=name;
      FREPAO(tasks).save(node.nodes.New());
   }
   bool load(TextNode &node)
   {
      if(TextNode *p=node.findNode("Name"))name=p.asText();
      Task t; FREPA(node.nodes)if(t.load(node.nodes[i]))Swap(tasks.New(), t);
      return name.is() && tasks.elms();
   }
   void save(MemPtr<Str> lines)C
   {
      FREPAO(tasks).save(lines);
   }
}
/******************************************************************************/
class SaveTaskClass : ClosableWindow
{
   Text     text;
   TextLine name;
   Button   yes, no;
   TaskName task_name;

   static void Yes(SaveTaskClass &st) {st.save();}
   static void No (SaveTaskClass &st) {st.button[2].push();}

   void create()
   {
      Gui+=super.create(Rect_C(0, 0, 1, 0.35), "Save Tasks").hide(); button[2].func(HideWindow, SCAST(Window, T)).show();
      T  +=yes  .create(Rect_D(clientWidth()*2/7, -clientHeight()+0.04, 0.25, 0.065), "Yes").func(Yes, T);
      T  +=no   .create(Rect_D(clientWidth()*5/7, -clientHeight()+0.04, 0.25, 0.065), "No" ).func(No , T);
      T  +=name .create(Rect_D(clientWidth()/2, no.rect().max.y+0.03, clientWidth()-0.06, 0.065));
      T  +=text .create(Rect(0, name.rect().max.y, clientWidth(), 0).extend(-0.01), "Would you like to save recorded tasks?"); text.auto_line=AUTO_LINE_SPACE_SPLIT;
   }
   void display(TaskName &task_name)
   {
      Swap(T.task_name, task_name);
      super.activate();
      name.set("Task Name").selectAll().activate();
   }
   void save()
   {
      if(name().is())
      {
         task_name.name=name();
         Swap(TaskNames.New(), task_name);
         RefreshTasks();
         button[2].push();
      }
   }
   virtual void update(C GuiPC &gpc)
   {
      super.update(gpc);

      if(Gui.window()==this)
      {
         if(Kb.k(KB_ENTER))
         {
            Kb.eatKey();
            save();
         }
      }
   }
}
SaveTaskClass SaveTask;
/******************************************************************************/
class ExecuteTaskClass : ClosableWindow
{
   Text   text;
   Button yes, no;
   UID    task_id;

   static void Yes(ExecuteTaskClass &et) {et.execute();}
   static void No (ExecuteTaskClass &et) {et.button[2].push();}

   void create()
   {
      Gui+=super.create(Rect_C(0, 0, 1, 0.30)).hide(); button[2].func(HideWindow, SCAST(Window, T)).show();
      T  +=yes  .create(Rect_D(clientWidth()*2/7, -clientHeight()+0.04, 0.25, 0.065), "Yes").func(Yes, T);
      T  +=no   .create(Rect_D(clientWidth()*5/7, -clientHeight()+0.04, 0.25, 0.065), "No" ).func(No , T);
      T  +=text .create(Rect(0, yes.rect().max.y, clientWidth(), 0).extend(-0.01), "Are you sure you want to execute selected task?"); text.auto_line=AUTO_LINE_SPACE_SPLIT;
   }
   void display(TaskName &task_name)
   {
      task_id=task_name.id;
      setTitle(S+"Execute Task \""+task_name.name+'"');
      super.activate();
   }
   void execute()
   {
      if(C TaskName *tn=FindTask(task_id))
         if(Pane *pane=(tn.usesRemote() ? ActiveRemotePane() : &LeftPane)) // execute tasks all on the same pane, if at least one uses remote, then use remote, otherwise use the LeftPane
      {
         tn.execute(*pane);
         button[2].push();
      }else Gui.msgBox("Can't execute task", "Task needs a remote pane, however there's no remote pane selected.");
   }
}
ExecuteTaskClass ExecuteTask;
/******************************************************************************/
class RenameTaskClass : ClosableWindow
{
   UID      task_id;
   TextLine textline;

   void create()
   {
      Gui+=super   .create(Rect_C(0, 0, 1, 0.14)).hide(); button[2].func(HideWindow, SCAST(Window, T)).show();
      T  +=textline.create(Rect  (0, -clientHeight(), clientWidth(), 0).extend(-0.01));
   }
   void display(TaskName &task_name)
   {
      task_id=task_name.id;
      textline.set(task_name.name).selectAll().activate();
      setTitle(S+"Rename Task \""+task_name.name+'"');
      super.activate();
   }
   virtual void update(C GuiPC &gpc)
   {
      super.update(gpc);

      if(Gui.window()==this)
      {
         if(Kb.k(KB_ENTER))
         {
            if(TaskName *tn=FindTask(task_id)){tn.name=textline(); RefreshTasks();}
            Kb.eatKey();
            button[2].push();
         }
      }
   }
}
RenameTaskClass RenameTask;
/******************************************************************************/
class DeleteTaskClass : ClosableWindow
{
   UID    task_id;
   Text   text;
   Button yes, no;

   static void Yes(DeleteTaskClass &dt) {dt.remove();}
   static void No (DeleteTaskClass &dt) {dt.button[2].push();}

   void create()
   {
      Gui+=super.create(Rect_C(0, 0, 1, 0.35)).hide(); button[2].func(HideWindow, SCAST(Window, T)).show();
      T  +=yes  .create(Rect_D(clientWidth()*2/7, -clientHeight()+0.04, 0.25, 0.065), "Yes").func(Yes, T);
      T  +=no   .create(Rect_D(clientWidth()*5/7, -clientHeight()+0.04, 0.25, 0.065), "No" ).func(No , T);
      T  +=text .create(Rect(0, yes.rect().max.y, clientWidth(), 0).extend(-0.01), "Are you sure you want to delete selected task?"); text.auto_line=AUTO_LINE_SPACE_SPLIT;
   }
   void display(C TaskName &task_name)
   {
      task_id=task_name.id;
      yes.activate();
      setTitle(S+"Delete Task \""+task_name.name+'"');
      super.activate();
   }
   void remove()
   {
      if(TaskName *tn=FindTask(task_id))
      {
         TaskNames.removeData(tn, true);
         RefreshTasks();
      }
      button[2].push();
   }
}
DeleteTaskClass DeleteTask;
/******************************************************************************/
class TaskOrganizerClass : ClosableWindow
{
   Region         names_region, region;
   List<TaskName> names_list;
   Memc<Str>      data;
   List<Str>      list;
   UID            cur_id=UIDZero, menu_id=UIDZero;
   Menu           menu;

   static void  RenameTask(TaskOrganizerClass &to) {if(TaskName *tn=FindTask(to.menu_id)). RenameTask.display(*tn);}
   static void ExecuteTask(TaskOrganizerClass &to) {if(TaskName *tn=FindTask(to.menu_id)).ExecuteTask.display(*tn);}
   static void  DeleteTask(TaskOrganizerClass &to) {if(TaskName *tn=FindTask(to.menu_id)). DeleteTask.display(*tn);}

   virtual TaskOrganizerClass& rect(C Rect &rect)
   {
      super.rect(rect);
      flt p=0.02, x=Min(0.5, clientWidth()*0.66);
      names_region.rect(Rect(p, -clientHeight()+p, x-p/2, -p));
      region      .rect(Rect(x+p/2, -clientHeight()+p, clientWidth()-p, -p));
      return T;
   }
   virtual Rect sizeLimit()C {Rect r=super.sizeLimit(); r.min.set(0.4, 0.3); return r;}
   void create()
   {
      Gui+=super.create("Task Organizer").hide(); button[2].func(HideWindow, SCAST(Window, T)).show(); flag|=WIN_RESIZABLE;
      T+=names_region.create();
      T+=region      .create();
      rect(Rect_C(0, 0, 2, 1.5));
      
      ListColumn lc[]=
      {
         ListColumn(MEMBER(TaskName, name), LCW_MAX_DATA_PARENT, "Name"),
      };
      names_region+=names_list.create(lc, Elms(lc), true); names_list.cur_mode=LCM_ALWAYS;

      ListColumn lc2[]=
      {
         ListColumn(DATA_STR, 0, SIZE(Str), LCW_MAX_DATA_PARENT, "Name"),
      };
      region+=list.create(lc2, Elms(lc2), true);

      Node<MenuElm> n;
      n.New().create("Rename" ,  RenameTask, T);
      n.New().create("Execute", ExecuteTask, T);
      n.New().create("Delete" ,  DeleteTask, T);
      Gui+=menu.create();
      menu.list.elmHeight(0.05).textSize(0, 1);
      menu.setData(n);
   }
   void refresh()
   {
      cur_id.zero(); 
      names_list.setData(TaskNames, null, true);
      data.clear();
      if(TaskName *tn=names_list())
      {
         cur_id=tn.id;
         tn.save(data);
      }
      list.setData(data);
   }
   virtual void update(C GuiPC &gpc)
   {
      super.update(gpc);
      if(gpc.visible && visible())
      {
         UID id =UIDZero; if(TaskName *tn=names_list())id=tn.id;
         if( id!=cur_id)refresh();
         if(Gui.ms()==&names_list)
         {
            if(Ms.bd(0))if(TaskName *tn=names_list()).ExecuteTask.display(*tn);
            if(Ms.bp(1))
            {
               names_list.cur=names_list.screenToVis(Ms.pos());
               if(TaskName *tn=names_list()){menu_id=tn.id; menu.activate().posRU(Ms.pos());}
            }
         }
      }
   }
}
TaskOrganizerClass TaskOrganizer;
/******************************************************************************/
TaskName   RecordedTasks;
Memc<TaskName> TaskNames;
/******************************************************************************/
TaskName* FindTask(C UID &id)
{
   REPA(TaskNames)if(TaskNames[i].id==id)return &TaskNames[i];
   return null;
}
/******************************************************************************/
void RefreshTasks()
{
   Node<MenuElm> m;
   m.New().create("Stop all in progress", StopAllTasks);
   m++;
   m.New().create("Record"  , RecordTasksToggle).flag(MENU_TOGGLABLE).setOn(RecordTasks);
   m.New().create("Organize", OrganizeTasks);
   if(TaskNames.elms())
   {
      m++;
      FREPA(TaskNames)m.New().create(TaskNames[i].name, ExecuteTasks, ptr(i));
   }
   Tasks.setData(m);
   TaskOrganizer.refresh();
}
/******************************************************************************/
