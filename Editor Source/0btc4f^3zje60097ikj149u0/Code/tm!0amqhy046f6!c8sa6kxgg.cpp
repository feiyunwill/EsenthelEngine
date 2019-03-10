/******************************************************************************/
Menu PaneMenu;
UID  PaneMenuID=UIDZero;
/******************************************************************************/
void HideWindow(Window &window)
{
   window.fadeOut();
   SetFocus();
}
/******************************************************************************/
class RenamePaneClass : ClosableWindow
{
   UID      pane_id;
   TextLine textline;

   void create()
   {
      Gui+=super   .create(Rect_C(0, 0, 1, 0.14), "Rename Connection").hide(); button[2].func(HideWindow, SCAST(Window, T)).show();
      T  +=textline.create(Rect  (0, -clientHeight(), clientWidth(), 0).extend(-0.01));
   }
   void display(C UID &pane_id)
   {
      if(Pane *pane=FindPane(pane_id))
         if(!pane.local)
      {
         int index=RightPanes.validIndex(pane); if(InRange(index, RightTabs))
         {
            T.pane_id=pane_id;
            textline.set(RightTabs.tab(index).text()).selectAll().activate();
            super.activate();
         }
      }
   }
   virtual void update(C GuiPC &gpc)
   {
      super.update(gpc);

      if(Gui.window()==this)
      {
         if(Kb.k(KB_ENTER))
         {
            if(Pane *pane=FindPane(pane_id))
            {
               int i=RightPanes.validIndex(pane);
               if(InRange(i, RightTabs))RightTabs.tab(i).text(textline());
            }
            Kb.eatKey();
            button[2].push();
         }
      }
   }
}
RenamePaneClass RenamePane;
/******************************************************************************/
class RemovePaneClass : ClosableWindow
{
   UID    pane_id;
   Text   text;
   Button yes, no;

   static void Yes(RemovePaneClass &rp) {rp.remove();}
   static void No (RemovePaneClass &rp) {rp.button[2].push();}

   void create()
   {
      Gui+=super.create(Rect_C(0, 0, 1, 0.35)).hide(); button[2].func(HideWindow, SCAST(Window, T)).show();
      T  +=yes  .create(Rect_D(clientWidth()*2/7, -clientHeight()+0.04, 0.25, 0.065), "Yes").func(Yes, T);
      T  +=no   .create(Rect_D(clientWidth()*5/7, -clientHeight()+0.04, 0.25, 0.065), "No" ).func(No , T);
      T  +=text .create(Rect(0, yes.rect().max.y, clientWidth(), 0).extend(-0.01), "Are you sure you want to remove this pane?"); text.auto_line=AUTO_LINE_SPACE_SPLIT;
   }
   void display(C UID &pane_id)
   {
      if(Pane *pane=FindPane(pane_id))
         if(!pane.local)
      {
         int index=RightPanes.validIndex(pane); if(InRange(index, RightTabs))
         {
            T.pane_id=pane_id;
            yes.activate();
            setTitle(S+"Remove Pane \""+RightTabs.tab(index).text()+'"');
            super.activate();
         }
      }
   }
   void remove()
   {
      RemovePaneDo(pane_id);
      button[2].push();
   }
}
RemovePaneClass RemovePane;
/******************************************************************************/
class RenameFileClass : ClosableWindow
{
   UID      pane_id;
   Str      full_path;
   TextLine textline;

   void create()
   {
      Gui+=super   .create(Rect_C(0, 0, 1, 0.14)).hide(); button[2].func(HideWindow, SCAST(Window, T)).show();
      T  +=textline.create(Rect  (0, -clientHeight(), clientWidth(), 0).extend(-0.01));
   }
   void display()
   {
      Pane &pane=ActivePane();
      if(Pane.Elm *elm=pane.list())
         if(elm.type==FSTD_DIR || elm.type==FSTD_FILE)
      {
         pane_id=pane.pane_id;
         full_path=pane.getPath()+elm.name;
         textline.set(elm.name).selectExtNot().activate();
         setTitle(S+((elm.type==FSTD_DIR) ? "Rename Folder" : "Rename File")+" \""+elm.name+'"');
         super.activate();
      }
   }
   virtual void update(C GuiPC &gpc)
   {
      super.update(gpc);

      if(Gui.window()==this)
      {
         if(Kb.k(KB_ENTER))
         {
            if(Pane *pane=FindPane(pane_id))pane.rename(full_path, textline());
            Kb.eatKey();
            button[2].push();
         }
      }
   }
}
RenameFileClass RenameFile;
/******************************************************************************/
class ReplaceFileClass : ClosableWindow
{
   UID       src_id, dest_id;
   Str       src_full_path, dest_full_path;
   Memc<Str> base_names;
   Text      text;
   Button    yes, no;

   static void Yes(ReplaceFileClass &df) {df.replace();}
   static void No (ReplaceFileClass &df) {df.button[2].push();}

   void create()
   {
      Gui+=super.create(Rect_C(0, 0, 1, 0.35)).hide(); button[2].func(HideWindow, SCAST(Window, T)).show();
      T  +=yes  .create(Rect_D(clientWidth()*2/7, -clientHeight()+0.04, 0.25, 0.065), "Yes").func(Yes, T);
      T  +=no   .create(Rect_D(clientWidth()*5/7, -clientHeight()+0.04, 0.25, 0.065), "No" ).func(No , T);
      T  +=text .create(Rect(0, yes.rect().max.y, clientWidth(), 0).extend(-0.01), "Are you sure you want to copy and replace destination with selected items?"); text.auto_line=AUTO_LINE_SPACE_SPLIT;
   }
   void display()
   {
      Pane &src=ActivePane();
      if(Pane *dest=InactivePane())if(dest!=&src && !(dest.local && src.local && EqualPath(dest.path(), src.path())))
      {
         int dirs=0, files=0;
         base_names.clear();
         REPA(src.list.sel)if(Pane.Elm *elm=src.list.absToData(src.list.sel[i]))
         {
            if(elm.type==FSTD_DIR ){dirs ++; base_names.add(elm.name);}
            if(elm.type==FSTD_FILE){files++; base_names.add(elm.name);}
         }
         if(base_names.elms())
         {
             src_id= src.pane_id;  src_full_path= src.getPath();
            dest_id=dest.pane_id; dest_full_path=dest.getPath();
            yes.activate();
            setTitle(((dirs==1 && files==0) ? "Replace Folder" : (dirs==0 && files==1) ? "Replace File" : "Replace Multiple Items"));
            if(base_names.elms()==1)title.space()+=S+'"'+base_names[0]+'"';
            super.activate();
         }
      }
   }
   void replace()
   {
      if(Pane *src =FindPane( src_id))
      if(Pane *dest=FindPane(dest_id))
      {
         // call on remote Pane so when it gets deleted, then the task gets deleted too
         if(!dest.local)dest.replace(false, *src , src_full_path, dest_full_path, base_names); // check this first, so if both 'src' and 'dest' are local then the 'src.replace' will get called
         else            src.replace(true , *dest, src_full_path, dest_full_path, base_names);
      }
      button[2].push();
   }
}
ReplaceFileClass ReplaceFile;
/******************************************************************************/
class CreateFolderClass : ClosableWindow
{
   UID      pane_id;
   Str      full_path;
   TextLine textline;

   void create()
   {
      Gui+=super   .create(Rect_C(0, 0, 1, 0.14), "Create Folder").hide(); button[2].func(HideWindow, SCAST(Window, T)).show();
      T  +=textline.create(Rect  (0, -clientHeight(), clientWidth(), 0).extend(-0.01));
   }
   void display()
   {
      Pane &pane=ActivePane();
      pane_id=pane.pane_id;
      full_path=pane.getPath();
      textline.clear().activate();
      super.activate();
   }
   virtual void update(C GuiPC &gpc)
   {
      super.update(gpc);

      if(Gui.window()==this)
      {
         if(Kb.k(KB_ENTER))
         {
            if(Pane *pane=FindPane(pane_id))pane.createDir(full_path, textline());
            Kb.eatKey();
            button[2].push();
         }
      }
   }
}
CreateFolderClass CreateFolder;
/******************************************************************************/
class DeleteFileClass : ClosableWindow
{
   UID       pane_id;
   Memc<Str> full_paths;
   Text      text;
   Button    yes, no;

   static void Yes(DeleteFileClass &df) {df.recycle();}
   static void No (DeleteFileClass &df) {df.button[2].push();}

   void create()
   {
      Gui+=super.create(Rect_C(0, 0, 1, 0.35)).hide(); button[2].func(HideWindow, SCAST(Window, T)).show();
      T  +=yes  .create(Rect_D(clientWidth()*2/7, -clientHeight()+0.04, 0.25, 0.065), "Yes").func(Yes, T);
      T  +=no   .create(Rect_D(clientWidth()*5/7, -clientHeight()+0.04, 0.25, 0.065), "No" ).func(No , T);
      T  +=text .create(Rect(0, yes.rect().max.y, clientWidth(), 0).extend(-0.01), "Are you sure you want to move selected items to the Recycle Bin?"); text.auto_line=AUTO_LINE_SPACE_SPLIT;
   }
   void display()
   {
      Pane &pane=ActivePane();
      int dirs=0, files=0;
      full_paths.clear();
      Str path=pane.getPath();
      REPA(pane.list.sel)if(Pane.Elm *elm=pane.list.absToData(pane.list.sel[i]))
      {
         if(elm.type==FSTD_DIR ){dirs ++; full_paths.add(path+elm.name);}
         if(elm.type==FSTD_FILE){files++; full_paths.add(path+elm.name);}
      }
      if(full_paths.elms())
      {
         pane_id=pane.pane_id;
         yes.activate();
         setTitle(((dirs==1 && files==0) ? "Delete Folder" : (dirs==0 && files==1) ? "Delete File" : "Delete Multiple Items"));
         if(full_paths.elms()==1)title.space()+=S+'"'+GetBase(full_paths[0])+'"';
         super.activate();
      }
   }
   void recycle()
   {
      if(Pane *pane=FindPane(pane_id))pane.recycle(full_paths);
      button[2].push();
   }
}
DeleteFileClass DeleteFile;
/******************************************************************************/
