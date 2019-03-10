/******************************************************************************/
class Pane
{
   class ElmBase : FileInfo
   {
      Str name; // base name

      bool save(File &f)C
      {
         f<<type<<size<<modify_time_utc<<name;
         return f.ok();
      }
      bool load(File &f)
      {
         attrib=0;
         f>>type>>size>>modify_time_utc>>name;
         return f.ok();
      }
   }
   class Elm : ElmBase
   {
      ImagePtr icon;
      Str      type_text;
      uint     list_type;
      DateTime modify_time_local;
      
      void validate()
      {
         list_type=(type==FSTD_FILE || type==FSTD_LINK);
         modify_time_local=modify_time_utc; modify_time_local.toLocal();
         switch(type)
         {
            case FSTD_DRIVE: icon= DriveIcon; type_text="Drive" ; break;
            case FSTD_DIR  : icon=FolderIcon; type_text="Folder"; break;
            default        : icon=  FileIcon; type_text=GetExt(name); break;
         }
      }
   }
   class Conn : Connection
   {
      bool sent=false, verified=false;
      Str  pass;

      bool connected ()C {return verified && state()==CONNECT_GREETED;}
      bool connecting()C {return state()==CONNECT_CONNECTING || state()==CONNECT_AWAIT_GREET || (state()==CONNECT_GREETED && !verified);}

      void del()
      {
         super.del();
         sent=verified=false;
         pass.clear();
      }
      void connect(C SockAddr &addr, C Str &pass)
      {
         del();
         clientConnectToServer(addr);
         T.pass=pass;
      }
      bool verify() // this is called on main thread, however secondary connection is additionally processed on a background thread, so process this only if not 'verified'
      {
         if(!verified && updateState(0))
         {
            if(!sent)
            {
               sent=true;
               File &f=data; f.reset().putByte(CMD_VERSION_CHECK).putStr(ConnectionName).cmpUIntV(ConnectionVersion).putStr(pass).pos(0); pass.clear();
               send(f);
            }
            if(receive(0))
            {
               Str message;
               File &f=data; if(f.getByte()==CMD_VERSION_CHECK && Equal(f.getStr(), ConnectionName, true))
               {
                  verified=f.getBool();
                  if(!verified)f>>message;
                  tcpNoDelay(true);
               }
               if(!verified) // if received something that wasn't correct version match then disconnect
               {
                  if(!message.is())message="Can't connect to File Browser using specified address";
                  Gui.msgBox(S, message);
                  return false;
               }
            }
         }
         if(state()==CONNECT_VERSION_CONFLICT)Gui.msgBox(S, "App Version conflict. Please upgrade your software.");
         return state()==CONNECT_GREETED || state()==CONNECT_AWAIT_GREET || state()==CONNECT_CONNECTING;
      }
      bool update(Pane &pane)
      {
         if(verified && receive(0))
         {
            File  &f=data;
            switch(f.getByte())
            {
               default: return false;

               case CMD_FILE_LIST:
               {
                  Str rel=f.getStr(); if(EqualPath(rel, pane.path(), true))
                  {
                     File temp; if(Decompress(f, temp, true))
                     {
                        temp.pos(0); Memc<Pane.ElmBase> elms; if(elms.load(temp))pane.setFileList(elms);
                     }
                  }
               }break;
            }
         }
         return true;
      }
   }

   bool       local=false, custom_cur_sel=false, refreshing=false, force_refresh=false;
   Memc<Elm>  elms;
   List<Elm>  list;
   Region     region;
   Progress   progress;
   Button     up;
   TextLine   path;
   ComboBox   jump;
   Str        last_cur;
   Memc<Str>  last_sel, jump_list;
   UID        pane_id=UIDZero;
   Text       t_addr, t_pass;
   TextLine     addr,   pass;
   Button     connect;
   Conn       conn[2]; // 0=main, 1=background
   MemberDesc md_size;
   Thread     thread;
   long       thread_progress[2]; // 0=current, 1=total
   MemcThreadSafe<Task> thread_tasks;
   Memc<Str>            thread_refresh_paths;

   static Str  FileSize   (C Elm &elm) {return (elm.type==FSTD_FILE) ? .FileSize(elm.size)+' ' : S;} // add space to increase padding between the next column
   static Str  DateModify (C Elm &elm) {return (elm.type==FSTD_FILE) ? elm.modify_time_local.asText() : S;}
   static void Up         (Pane &pane) {pane.path.set(GetPath(pane.path()));}
   static void PathChanged(Pane &pane) {pane.list.scrollTo(0, true); pane.refresh();}
   static void Jump       (C Str &path, Pane &pane) {pane.jumpDo(path);}
   static void Connect    (Pane &pane) {pane.connectDo();}
   static void SelChanged (Pane &pane) {pane.custom_cur_sel=false;}
   static bool ThreadFunc (Thread &thread) {return ((Pane*)thread.user).threadUpdate();}

  ~Pane() {thread.del();} // delete thread before anything else
   Pane() {REPAO(thread_progress)=0;}

   Str getPath()C {return Str(path()).tailSlash(true);}

   bool connecting  ()C {if(local)return false; REPA(conn)if( conn[i].connecting())return true ; return false;}
   bool connected   ()C {if(local)return true ; REPA(conn)if(!conn[i].connected ())return false; return true ;}
   void connectClear()  {stopTasks(); REPAO(conn).del(); refreshing=false;}
   void connectDo   ()
   {
      if(connecting())connectClear();else // stop connect attempt
      { // connect
         connectClear();
         SockAddr sa; if(!sa.fromText(addr())){Gui.msgBox(S, "Invalid Address"); return;}
         REPAO(conn).connect(sa, pass());
         force_refresh=true;
      }
   }
   void threadTask(C Task &task)
   {
      thread_tasks.add(task);
      if(!thread.active())thread.create(ThreadFunc, this);
   }
   void stopTasks()
   {
      thread.del(); thread_tasks.clear(); REPAO(thread_progress)=0;
   }

   void create(GuiObj &parent, bool local=false)
   {
      T.local=local;
      pane_id.randomize();
      ListColumn lc[]=
      {
        ListColumn(MEMBER(Elm, icon), 0.05, S), // 0
        ListColumn(MEMBER(Elm, name), 0.70, "Name"), // 1
        ListColumn(FileSize  , 0.20, "Size"), // 2
        ListColumn(DateModify, 0.40, "Date Modified"), // 3
        ListColumn(MEMBER(Elm, type_text), 0.15, "Type"), // 4
      };
      lc[2].text_align=-1;
      lc[2].sort=&md_size;
      md_size.set(MEMBER(Elm, size));
      parent+=region.create();
      parent+=progress.create().hide();
      region+=list.create(lc, Elms(lc)).selChanged(SelChanged, T);
      list.flag|=LIST_RESIZABLE_COLUMNS|LIST_SORTABLE|LIST_SEARCHABLE|LIST_MULTI_SEL|LIST_TYPE_SORT;
      list.setElmType(MEMBER(Elm, list_type));
      list.sort_column[0]=1;
      parent+=up  .create("Up").func(Up, T, true).focusable(false);
      parent+=path.create().func(PathChanged, T, true);
      parent+=jump.create(); jump.flag=COMBOBOX_CONST_TEXT;
      if(!local)
      {
         parent+=t_addr .create("Connection Address" ); parent+=addr.create();
         parent+=t_pass .create("Connection Password"); parent+=pass.create();
         parent+=connect.create().func(Connect, T).focusable(false);
      }
      refresh();
   }

   void clearCurSel()
   {
      custom_cur_sel=false;
      last_cur.clear();
      last_sel.clear();
   }
   void setCurSel(C Str &base_name)
   {
      custom_cur_sel=true;
      last_cur=base_name;
      last_sel.clear(); if(base_name.is())last_sel.add(base_name);
   }
   void storeCurSel()
   {
      clearCurSel();
      if(Elm *elm=list())last_cur=elm.name;
      REPA(list.sel)if(Elm *elm=list.absToData(list.sel[i]))last_sel.add(elm.name);
   }
   void restoreCurSel()
   {
      if(last_cur.is())
      {
         REPA(list)if(Elm *elm=list.visToData(i))if(elm.name==last_cur){list.cur=i; list.scrollTo(i, true, custom_cur_sel); break;}
      }
      if(last_sel.elms())
      {
         last_sel.sort(CompareCI);
         REPA(list)if(Elm *elm=list.absToData(i))if(last_sel.binaryHas(elm.name, CompareCI))list.sel.add(i);
      }
   }

   void setFileList(C MemPtr<ElmBase> &src)
   {
      refreshing=false;
      if(!custom_cur_sel)storeCurSel();
      SCAST(Memc<ElmBase>, elms)=src;
      REPAO(elms).validate();
      list.setData(elms);
      restoreCurSel();
   }
   void jumpDo(C Str &jump)
   {
      if(jump=="Desktop" )path.set(SystemPath(SP_DESKTOP));else
      if(jump=="OneDrive")path.set(SystemPath(SP_ONE_DRIVE));else
      if(jump=="Add to Favorites")
      {
         Str p=path(); p.tailSlash(false);
         REPA(jump_list)if(EqualPath(jump_list[i], p))return;
         jump_list.add(p); refreshJumpList();
      }else
      if(jump=="Remove from Favorites")
      {
         Str p=path(); p.tailSlash(false);
         REPA(jump_list)if(EqualPath(jump_list[i], p)){jump_list.remove(i, true); refreshJumpList();}
      }else
      path.set(jump);
   }
   void refreshJumpList()
   {
      Node<MenuElm> n;
      if(local)
      {
         if(SystemPath(SP_DESKTOP  ).is())n.New().create("Desktop");
         if(SystemPath(SP_ONE_DRIVE).is())n.New().create("OneDrive");
         if(n.children.elms())n++;
      }
      if(jump_list.elms())
      {
         FREPA(jump_list)n.New().create(jump_list[i]);
         n++;
      }
      n.New().create("Add to Favorites");
      n.New().create("Remove from Favorites");
      jump.setData(n).menu.func(Jump, T);
   }
   void refresh()
   {
      if(local)
      {
         Memc<ElmBase> elms;
         GetFileList(path(), elms);
         setFileList(elms);
         force_refresh=false;
      }else
      {
         list.clear(QUIET);
         elms.clear();
         if(connected())
         {
            File f; f.writeMem().putByte(CMD_FILE_LIST).putStr(path()).pos(0);
            conn[0].send(f);
            refreshing=true;
            force_refresh=false;
         }
      }
   }

   void resize(bool left)
   {
      const flt h=0.07;
      Rect rect(left ? -D.w()+Padd : Padd/2, RenameButton.rect().max.y+Padd+0.01, left ? Padd/-2 : D.w()-Padd, RightTabs.rect().min.y); // reserve extra space at the bottom for progress bar
      up  .rect(Rect_LU(rect.min.x, rect.max.y, 0.1, h));
      jump.rect(Rect_RU(rect.max.x, rect.max.y, h, h));
      path.rect(Rect(up.rect().rd(), jump.rect().lu()));
      rect.max.y=up.rect().min.y-Padd;
      region.rect(rect);
      progress.rect(Rect_LU(rect.ld(), rect.w(), 0.02));
      Vec2 pos=rect.center(); pos.y+=0.1;
      t_addr.pos(Vec2(pos.x-0.22, pos.y)); addr.rect(Rect_L(pos.x, pos.y, 0.4, 0.06)); pos.y-=0.07;
      t_pass.pos(Vec2(pos.x-0.22, pos.y)); pass.rect(Rect_L(pos.x, pos.y, 0.4, 0.06)); pos.y-=0.07;
      connect.rect(Rect_U(pos, 0.3, 0.06));
   }
   void enter()
   {
      if(Elm *elm=list())
      {
         Str full=getPath()+elm.name;
         if(elm.type==FSTD_DIR  || elm.type==FSTD_DRIVE){setCurSel(S); path.set(full);}else
         if(elm.type==FSTD_FILE || elm.type==FSTD_LINK )if(local)Run(full);
      }
   }
   void back()
   {
      setCurSel(GetBase(path()).tailSlash(false));
      path.set(GetPath(path()));      
   }
   bool execute(Task &task, bool main=true)
   {
      Conn &conn=T.conn[main ? 0 : 1];
      File &f=conn.data;
      switch(task.type)
      {
         case CMD_RENAME:
         {
            if(task.src_local)
            {
               FREPA(task.names)
               {
                C Str2 &name=task.names[i];
                  if(!FExistSystem(name.dest) || EqualPath(name.src, name.dest) && !Equal(GetBase(name.src), GetBase(name.dest), true)) // dest doesn't exist, or the same path, but name with different case
                  {
                     FCreateDirs(GetPath  (name.dest));
                     FRename    (name.src, name.dest );
                  }else Gui.msgBox(S, "Can't rename because target already exists");
               }
            }else
            {
               f.reset().putByte(task.type); task.saveNames(f); f.pos(0); conn.send(f);
            }
         }break;

         case CMD_CREATE_DIR:
         {
            if(task.src_local)
            {
               FREPA(task.names)FCreateDirs(task.names[i].src);
            }else
            {
               f.reset().putByte(task.type); task.saveNames(f); f.pos(0); conn.send(f);
            }
         }break;

         case CMD_DELETE:
         {
            if(task.src_local)
            {
               FREPA(task.names)FRecycle(task.names[i].src);
            }else
            {
               f.reset().putByte(task.type); task.saveNames(f); f.pos(0); conn.send(f);
            }
         }break;
         
         case CMD_REPLACE:
         {
            if(main)
            {
               if(!connected()){Gui.msgBox("Can't replace", "Pane is currently disconnected."); return false;}
               if(task.src_local || task.dest_local)threadTask(task); // this is a slow operation so needs to be processed on secondary thread
            }else
            {
               FREPA(task.names)
               {
                  if(thread.wantStop())break;
                  Str2 &name=task.names[i];
                  if(task.dest_local)thread_refresh_paths.binaryInclude(GetPath(name.dest).tailSlash(false), ComparePathCI);
                  if(task. src_local && task.dest_local) // both are local
                  {
                     FileInfoSystem src(name.src), dest(name.dest);
                     if(src.type==FSTD_FILE || src.type==FSTD_LINK)
                     {
                        if(src!=dest)
                        {
                           if(dest.type==FSTD_DIR && !RecycleLoud(name.dest))break;
                           if(!SafeCopyLoud(name.src, name.dest))break;
                        }
                     }else
                     {
                        FileLister src_files(name.src , thread),
                                  dest_files(name.dest, thread);
                        if(dest.type!=FSTD_DIR && dest.type!=FSTD_DRIVE)
                        {
                           if(dest.type && !RecycleLoud(name.dest))break;
                           if(!CreateDirLoud(name.dest))break;
                        }
                        if(!thread.wantStop())
                        {
                           Memc<int> dest_remove, src_copy;
                           Patcher.Compare(src_files.Files(), dest_files.Files(), dest_remove, src_copy);
                           long total=dest_remove.elms()+src_copy.elms()+1; FREPA(src_copy)total+=src_files.files[src_copy[i]].file_size; // total=number of operations + 1 (in progress to prevent progressbar disappearing when there are no ops) + data size
                           thread_progress[0]=0;
                           thread_progress[1]=total;

                           FREPA(dest_remove) // recycle in order
                           {
                              if(thread.wantStop())break;
                              Str path=dest_files.path+dest_files.files[dest_remove[i]].full_name;
                              if(FExistSystem(path) && !RecycleLoud(path))break; // do an extra check for 'FExistSystem' first, in case the file belonged to a folder which already got recycled
                              thread_progress[0]++;
                           }
                           FREPA(src_copy) // copy in order
                           {
                              if(thread.wantStop())break;
                            C Patcher.LocalFile &file=src_files.files[src_copy[i]];
                              Str dest=dest_files.path+file.full_name;
                              if(file.type==Patcher.LocalFile.SYSTEM_DIR){if(!CreateDirLoud(dest))break;}
                              else                                        if(! SafeCopyLoud(src_files.path+file.full_name, dest))break;
                              thread_progress[0]+=1+file.file_size;
                           }
                        }
                     }
                  }else
                  if(task.src_local || task.dest_local) // one is local, other one is remote
                  {
                     // send first
                     f.reset().putByte(CMD_FILE_LIST_RECURSIVE).putStr(task.src_local ? name.dest : name.src).pos(0); if(!conn.send(f))return false;

                     // process locally
                   C Str       &local_path=(task.src_local ? name.src : name.dest);
                     FileList  remote_files;
                     FileLister local_files(local_path, thread);

                     // receive
                     for(;;)
                     {
                        if(thread.wantStop() || !conn.updateState(0))return false;
                        if(conn.receive(1))
                        {
                           if(f.getByte()!=CMD_FILE_LIST_RECURSIVE)return false;
                           File temp; if(!Decompress(f, temp, true))return false;
                           temp.pos(0); if(!remote_files.load(temp))return false;
                           if(!EqualPath(remote_files.name, task.src_local ? name.dest : name.src))return false;
                           break;
                        }
                     }

                     // process
                     FileInfoSystem local(local_path); // get local file info
                   C FileInfo      &remote=remote_files.fi,
                                   &src   =(task.src_local ? local  : remote),
                                   &dest  =(task.src_local ? remote : local );
                     Task recycle(CMD_DELETE                            , task.src_local),
                       create_dir(CMD_CREATE_DIR                        , task.src_local),
                   transfer_files(task.src_local ? CMD_REPLACE : CMD_GET, task.src_local, task.dest_local);
                     if(src.type==FSTD_FILE || src.type==FSTD_LINK)
                     {
                        if(src!=dest)
                        {
                           if(dest.type==FSTD_DIR)
                           {
                              if(task.src_local)recycle.names.New().set(name.dest);
                              else          if(!RecycleLoud(name.dest))break;
                           }
                           transfer_files.names.New().set(name.src, name.dest);
                           thread_progress[0]=0;
                           thread_progress[1]=1+1+src.size; // total=number of operations + 1 (in progress to prevent progressbar disappearing when there are no ops) + data size
                        }
                     }else
                     {
                        name.src .tailSlash(true);
                        name.dest.tailSlash(true);
                        Memc<Patcher.LocalFile> & src_files=(task.src_local ?  local_files.files : remote_files.files),
                                                &dest_files=(task.src_local ? remote_files.files :  local_files.files);

                        if(dest.type!=FSTD_DIR && dest.type!=FSTD_DRIVE)
                        {
                           // create dir
                           if(task.dest_local) // locally
                           {
                              if(dest.type && !RecycleLoud(name.dest))break;
                              if(!CreateDirLoud(name.dest))break;
                           }else // remotely
                           {
                              if(dest.type)recycle.names.New().set(name.dest);
                              create_dir.names.New().set(name.dest);
                           }
                        }
                        Memc<int> dest_remove, src_copy;
                        Patcher.Compare(src_files, dest_files, dest_remove, src_copy);
                        long total=dest_remove.elms()+src_copy.elms()+1; FREPA(src_copy)total+=src_files[src_copy[i]].file_size; // total=number of operations + 1 (in progress to prevent progressbar disappearing when there are no ops) + data size
                        thread_progress[0]=0;
                        thread_progress[1]=total;

                        // recycle
                        FREPA(dest_remove) // recycle in order
                        {
                           if(thread.wantStop())break;
                           Patcher.LocalFile &dest_file=dest_files[dest_remove[i]];
                           Str path=name.dest+dest_file.full_name;
                           if(task.dest_local) // locally
                           {
                              if(FExistSystem(path) && !RecycleLoud(path))break; // do an extra check for 'FExistSystem' first, in case the file belonged to a folder which already got recycled
                           }else // remotely
                           {
                              recycle.names.New().set(path);
                           }
                           thread_progress[0]++;
                        }

                        // create folders
                        FREPA(src_copy) // create in order
                        {
                           if(thread.wantStop())break;
                           Patcher.LocalFile &src_file=src_files[src_copy[i]];
                           if(src_file.type==Patcher.LocalFile.SYSTEM_DIR)
                           {
                              Str path=name.dest+src_file.full_name;
                              if(task.dest_local) // locally
                              {
                                 if(!CreateDirLoud(path))break;
                              }else // remotely
                              {
                                 create_dir.names.New().set(path);
                              }
                           }
                           thread_progress[0]++;
                        }

                        // transfer files
                        FREPA(src_copy) // create in order
                        {
                           if(thread.wantStop())break;
                           Patcher.LocalFile &src_file=src_files[src_copy[i]];
                           if(src_file.type!=Patcher.LocalFile.SYSTEM_DIR)
                              transfer_files.names.New().set(name.src+src_file.full_name, name.dest+src_file.full_name);
                        }
                     }

                     // perform
                     if(recycle.names.elms())
                     {
                        f.reset().putByte(recycle.type); recycle.saveNames(f); f.pos(0); conn.send(f);
                     }
                     if(create_dir.names.elms())
                     {
                        f.reset().putByte(create_dir.type); create_dir.saveNames(f); f.pos(0); conn.send(f);
                     }
                     if(transfer_files.names.elms())
                     {
                        if(task.src_local) // send to remote
                        {
                           bool ok=SendFiles(transfer_files.names, conn, thread_progress[0], thread, true);
                           // wait until remote finishes (do this always, so the other side can reset 'all_ok')
                           f.reset().putByte(CMD_REPLY).pos(0); conn.send(f);
                           for(; !thread.wantStop() && conn.updateState(0); )if(conn.receive(1))
                           {
                              ok&=(conn.data.getByte()==CMD_REPLY && conn.data.getBool());
                              break;
                           }
                           if(!ok)Gui.msgBox(S, "Error transferring files");
                        }else // receive from remote
                        {
                           f.reset().putByte(transfer_files.type);
                           f.cmpUIntV(transfer_files.names.elms()); FREPA(transfer_files.names)f<<transfer_files.names[i].src;
                           f.pos(0); conn.send(f);
                           FREPA(transfer_files.names)
                           {
                            C Str2 &tf=transfer_files.names[i];
                            again:
                              if(thread.wantStop() || !conn.updateState(0))break;
                              if(!conn.receive(1))goto again;
                              byte b=conn.data.getByte(); if(b!=CMD_REPLACE)return false;
                              IOFileData file; if(!file.load(conn.data))return false;
                              if(!EqualPath(tf.src, file.name, true))return false;
                              if(!file.append(tf.dest, conn.data)){Gui.msgBox(S, S+"Error transferring file\n\""+tf.dest+'"'); return false;}
                              thread_progress[0]+=file.uncompressed_size;
                              if(file.finished || file.error)
                              {
                                 thread_progress[0]++;
                                 continue;
                              }
                              goto again;
                           }
                        }
                     }
                  }
               }
            }
         }break;
      }
      return true;
   }
   void rename(C Str &src_full, C Str &dest)
   {
      if(src_full.is() && dest.is())
      {
         Str dest_full=ExpandPath(GetPath(src_full), dest);
         if(!EqualPath(src_full, dest_full) || !Equal(GetBase(src_full), GetBase(dest_full), true))
         {
            Task task(CMD_RENAME, local); task.names.New().set(src_full, dest_full);
            if(RecordTasks)Swap(RecordedTasks.tasks.New(), task);else
            {
               setCurSel(GetBase(dest_full));
               path.set(GetPath(dest_full), QUIET);
               execute(task);
               refresh();
            }
         }
      }
   }
   void createDir(C Str &full_path, C Str &name)
   {
      if(name.is())
      {
         Str n=ExpandPath(full_path, name);
         Task task(CMD_CREATE_DIR, local); task.names.New().set(n);
         if(RecordTasks)Swap(RecordedTasks.tasks.New(), task);else
         {
            setCurSel(GetBase(n));
            path.set(GetPath(n), QUIET);
            execute(task);
            refresh();
         }
      }
   }
   void recycle(C MemPtr<Str> &full_paths)
   {
      if(full_paths.elms())
      {
         Task task(CMD_DELETE, local); FREPA(full_paths)task.names.New().set(full_paths[i]); // add in order
         if(RecordTasks)Swap(RecordedTasks.tasks.New(), task);else
         {
            // get next (or previous) item that's not being deleted
            Str next, path=getPath();
                          for(int i=list.cur; i<list.visibleElms(); i++)if(Elm *elm=list.visToData(i))if(!full_paths.has(path+elm.name)){next=elm.name; break;}
            if(!next.is())for(int i=list.cur; i>=0                ; i--)if(Elm *elm=list.visToData(i))if(!full_paths.has(path+elm.name)){next=elm.name; break;}
            setCurSel(next);
            execute(task);
            refresh();
         }
      }
   }
   void replace(bool this_is_src, C Pane &pane, C Str &src_full_path, C Str &dest_full_path, C MemPtr<Str> &base_names)
   {
      Task task(CMD_REPLACE, this_is_src ? local : pane.local, this_is_src ? pane.local : local);
      FREPA(base_names)task.names.New().set(src_full_path+base_names[i], dest_full_path+base_names[i]);
      if(RecordTasks)Swap(RecordedTasks.tasks.New(), task);else execute(task);
   }
   void setConnectedVisibility()
   {
      bool connected=T.connected(), connecting=T.connecting();
      list   .visible(connected);
      path   .visible(connected);
      jump   .visible(connected);
      up     .visible(connected);
      t_addr .hidden (connected); addr.hidden(connected).disabled(connecting);
      t_pass .hidden (connected); pass.hidden(connected).disabled(connecting);
      connect.hidden (connected);
      region .skin   (connected ? Gui.skin : &TransparentSkin, false);
      if(connect.visible())connect.text=(connecting ? "Stop" : "Connect");
   }
   void refreshLocalPath(C MemPtr<Str> &paths)
   {
      if(local)REPA(paths)if(StartsPath(path(), paths[i])){force_refresh=true; break;}
   }
   void update(bool left)
   {
      list.skin((left==LeftFocus) ? null : &TransparentSkin);
      if(Ms.bd(0) && Gui.ms()==&list){enter(); Ms.eat(0);}
      if(Ms.bp(1) && Gui.ms()==&list)back();
      if(Gui.kb()==&list)
      {
         if(Kb.k(KB_BACK))back();
         if(Kb.kf(KB_ENTER) || Kb.kf(KB_NPENTER))enter();
      }
      if(!local)
      {
         if(Gui.kb()==&addr || Gui.kb()==&pass)
         {
            if(Kb.kf(KB_ENTER) || Kb.kf(KB_NPENTER))connect.push();
         }

         REPA(conn)if(!conn[i].verify())connectClear();
         if(!conn[0].update(T))connectClear();
         setConnectedVisibility();
      }
      if(force_refresh)refresh();

      long progress=thread_progress[0], total=thread_progress[1]+refreshing;
      T.progress.set(progress, total).visible(total>0);
   }
   bool threadUpdate()
   {
      if(thread_tasks.elms())
      {
         thread_progress[0]=0; thread_progress[1]=1; // set as in-progress
         Task task;
         thread_tasks.  lock(); Swap(task, thread_tasks.lockedElm(0)); thread_tasks.remove(0, true);
         thread_tasks.unlock();
         if(!execute(task, false))return false;
         if(!thread_tasks.elms()) // if processed all tasks
         {
            if(thread_refresh_paths.elms())
            {
               RefreshLocalPaths.  lock(); FREPA(thread_refresh_paths)RefreshLocalPaths.binaryInclude(thread_refresh_paths[i], ComparePathCI);
               RefreshLocalPaths.unlock(); thread_refresh_paths.clear();
            }
            force_refresh=true; // refresh
            thread_progress[0]=thread_progress[1]=0; // set as finished
         }
      }else Time.wait(1);
      return true;
   }
   void save(TextNode &node)
   {
      node.getNode("Path").setValue(path());
      if(jump_list.elms())
      {
         TextNode &jump=node.getNode("Jump");
         FREPA(jump_list)jump.nodes.New().setValue(jump_list[i]);
      }
      if(!local)
      {
         node.nodes.New().set("Address" , addr());
         node.nodes.New().set("Password", pass());
      }
   }
   void load(TextNode &node)
   {
      if(TextNode *n=node.findNode("Path"))path.set(n.asText());
      if(TextNode *n=node.findNode("Jump"))FREPA(n.nodes)jump_list.add(n.nodes[i].asText());

      if(TextNode *n=node.findNode("Address" ))addr.set(n.asText());
      if(TextNode *n=node.findNode("Password"))pass.set(n.asText());
   }
}
/******************************************************************************/
