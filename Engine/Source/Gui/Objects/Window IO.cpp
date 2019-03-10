/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************

   Here FSTD_LINK is used for folders used as files.

/******************************************************************************/
#define QPS_DATA         "Data"
#define QPS_DESKTOP      "Desktop"
#define QPS_ONEDRIVE     "OneDrive"
#define QPS_FAVORITES    "Favorites:"
#define QPS_RECENT       "Recent:"
#define QPS_ADD_FAVORITE "Add to Favorites"
#define QPS_REM_FAVORITE "Remove from Favorites"
#define QPS_MAX_RECENT   8
/******************************************************************************/
static Str FileSize(C WindowIO::File &f)
{
   switch(f.type)
   {
      case FSTD_FILE: return TextInt((f.size+1023)>>10, -1, 3)+" KB";
      case FSTD_LINK: return "Unknown";
      default       : return S;
   }
}
static MemberDesc win_io_file_size_sort(MEMBER(WindowIO::File, size));
Memc<Str> WindowIOFavorites, WindowIORecents;
/******************************************************************************/
void WindowIORecent(Str path)
{
   if(path.is())REPA(WindowIORecents)if(EqualPath(WindowIORecents[i], path))
   {
      REPD(j, i)Swap(WindowIORecents[j], WindowIORecents[j+1]); // move to 1st position
      return;
   }
   WindowIORecents.NewAt(0)=path;
   WindowIORecents.setNum(Min(QPS_MAX_RECENT, WindowIORecents.elms())); // limit amount of elements
}
/******************************************************************************/
static void ChangedText    (WindowIO &wio) {wio.setBar         ();}
static void OK             (WindowIO &wio) {wio.Ok             ();}
static void Cancel         (WindowIO &wio) {wio.hide           ();}
static void CreateDir      (WindowIO &wio) {wio.createDir      ();}
static void Up             (WindowIO &wio) {wio.back           ();}
static void Rename         (WindowIO &wio) {wio.renameDo       ();}
static void RemoveAsk      (WindowIO &wio) {wio.removeAsk      ();}
static void RemoveDo       (WindowIO &wio) {wio.removeDo       ();}
static void RemoveCancel   (WindowIO &wio) {wio.removeCancel   ();}
static void OverwriteDo    (WindowIO &wio) {wio.overwriteDo    ();}
static void OverwriteCancel(WindowIO &wio) {wio.overwriteCancel();}
static void Explore        (WindowIO &wio) {wio.exploreDo      ();}
/******************************************************************************/
// QUICK PATH SELECTOR
/******************************************************************************/
#if IOS // on iOS we can only write to "SystemPath(SP_APP_DATA)"
static Str QPSShortPath(C Str &path) {return SkipStartPath(path, SystemPath(SP_APP_DATA));}
static Str QPSFullPath (C Str &path) {return (path.is() && !FullPath(path)) ? SystemPath(SP_APP_DATA).tailSlash(true)+path : path;}
#else
static Str QPSShortPath(C Str &path) {return SkipStartPath(path, DataPath());}
static Str QPSFullPath (C Str &path) {return (path.is() && !FullPath(path)) ? DataPath()+path : path;}
#endif
static void PathChanged(C Str &path, QuickPathSelector &qps)
{
   if(path.is())
   {
      if(path==QPS_DATA        )qps.setPath(DataPath());else
      if(path==QPS_DESKTOP     )qps.setPath(SystemPath(SP_DESKTOP  ));else
      if(path==QPS_ONEDRIVE    )qps.setPath(SystemPath(SP_ONE_DRIVE));else
      if(path==QPS_ADD_FAVORITE)
      {
         Str path=qps.getPath(); if(path.is())
         {
            if(!QPSShortPath(path).is())return;
            REPA(WindowIOFavorites)if(EqualPath(WindowIOFavorites[i], path))return;
            WindowIOFavorites.add(path);
         }
      }else
      if(path==QPS_REM_FAVORITE)
      {
         Str path=qps.getPath(); if(path.is())
         {
            REPA(WindowIOFavorites)if(EqualPath(WindowIOFavorites[i], path))WindowIOFavorites.remove(i, true);
         }
      }else
      if(path!=QPS_FAVORITES && path!=QPS_RECENT)qps.setPath(QPSFullPath(path)); // custom path
   }
}
static void QPSChangedButton(QuickPathSelector &qps)
{
   if(qps.Button::operator()())qps.setData(); // set most recent data
   if(qps._func)qps._func(&qps);  // call stored callback
}
ComboBox& QuickPathSelector::create()
{
   super::create();
   menu_align=-1;
  _func =Button::_func; // store previous copy of the button function
   Button::func(QPSChangedButton, T);
   return T;
}
void QuickPathSelector::setData()
{
   Node<MenuElm> n;
   {
      if(  DataPath(            ).is())n+=QPS_DATA;
      if(SystemPath(SP_DESKTOP  ).is())n+=QPS_DESKTOP;
      if(SystemPath(SP_ONE_DRIVE).is())n+=QPS_ONEDRIVE;
      Bool header=false;
      FREPA(WindowIOFavorites)
      {
         Str p=QPSShortPath(WindowIOFavorites[i]);
         if( p.is())
         {
            if(!header){header=true; if(n.children.elms())n++; Node<MenuElm> &c=(n+=QPS_FAVORITES); c.flag(MENU_NOT_SELECTABLE);}
            n+=p;
         }
      }
      header=false;
      FREPA(WindowIORecents)
      {
         Str p=QPSShortPath(WindowIORecents[i]);
         if( p.is())
         {
            if(!header){header=true; if(n.children.elms())n++; Node<MenuElm> &c=(n+=QPS_RECENT); c.flag(MENU_NOT_SELECTABLE);}
            n+=p;
         }
      }
      if(n.children.elms())n++;
      n+=QPS_ADD_FAVORITE;
      n+=QPS_REM_FAVORITE;
   }
   super::setData(n).menu.func(PathChanged, T);
}
     WindowIO::QPS::QPS(WindowIO &win_io) : win_io(win_io) {}
Str  WindowIO::QPS::getPath(           ) {return win_io.path()+win_io.subPath();}
void WindowIO::QPS::setPath(C Str &path) {win_io.path(S, path); win_io.getList();}
/******************************************************************************/
// WINDOW IO
/******************************************************************************/
void WindowIO::zero()
{
   path_mode=WIN_IO_PM_FULL;
   ext_mode=WIN_IO_EXT_ALWAYS;
  _border=0.06f;
  _mode=WIN_IO_LOAD;
  _dir_operate=false;
  _func_user=null;
  _load=null;
  _save=null;
}
WindowIO::WindowIO() : quick(T) {zero();}
WindowIO& WindowIO::del()
{
      rename_window.del();
      remove_window.del();
   overwrite_window.del();
       _ext.del();
  _ext_desc.del();
  _dot_exts.del();
      _path.del();
  _sub_path.del();
   _op_name.del();
       file.del();
   super::del(); zero(); return T;
}
WindowIO& WindowIO::create(C Str &ext, C Str &path, C Str &sub_path, void (*load)(C Str &name, Ptr user), void (*save)(C Str &name, Ptr user), Ptr user)
{
   del();

   ListColumn win_io_file_column[]=
   {
      ListColumn(MEMBER(WindowIO::File, name), 0.00f, MLTC(u"Name", PL,u"Nazwa"  )), // 0 
      ListColumn(FileSize                    , 0.29f, MLTC(u"Size", PL,u"Rozmiar")), // 1
   };
   win_io_file_column[1].sort=&win_io_file_size_sort;

   Gui   +=super    ::create().hide(); button[2].show(); flag|=WIN_RESIZABLE;
   T     +=region    .create();
   T     +=textline  .create().func(ChangedText, T).desc(MLTC(u"Name", PL,u"Nazwa"));
   T     +=ok        .create(MLTC(u"OK"              , PL,u"OK"            )).func(OK       , T);
   T     +=cancel    .create(MLTC(u"Cancel"          , PL,u"Anuluj"        )).func(Cancel   , T);
   T     +=create_dir.create(MLTC(u"Create Directory", PL,u"Utwórz Katalog")).func(CreateDir, T).desc(MLTC(u"Create Directory using name specified above", PL,u"Utwórz Katalog na podstawie podanej nazwy"));
   T     +=up        .create(MLTC(u"Up"              , PL,u"Wstecz"        )).func(Up       , T);
   T     +=rename    .create(MLTC(u"Rename"          , PL,u"Zmień Nazwę"   )).func(Rename   , T).desc(MLTC(u"Rename (F2)", PL,u"Zmień nazwę (F2)"));
   T     +=remove    .create(MLTC(u"Delete"          , PL,u"Usuń"          )).func(RemoveAsk, T).desc(MLTC(u"Move to Recycle Bin (Del)", PL,u"Przenieś do kosza (Del)"));
#if DESKTOP
   T     +=explore   .create(MLTC(u"Explore"         , PL,u"Eksploruj"     )).func(Explore  , T).desc(MLTC(u"Open in Windows Explorer (F4)", PL,u"Otwórz w Eksploratorze Windows (F4)"));
#endif
   T     +=quick     .create(                                             ).desc("Quick select path by using Favorites");
   region+=list      .create(win_io_file_column, Elms(win_io_file_column)).setElmType(MEMBER(WindowIO::File, type)); list.sort_column[0]=0; list.flag|=LIST_TYPE_SORT|LIST_SCALABLE; list.cur_mode=LCM_ALWAYS; list.zoom_min=1/1.2f; list.zoom_max=1.2f;
   Node<MenuElm> node; T+=menu.create(node).hide();

   Gui+=rename_window.create().level(1).rect(Rect_C(0, 0, 1.2f, 0.14f)).hide(); rename_window.button[2].show();
   rename_window+=rename_textline.create(Rect_LU(0, 0, rename_window.clientWidth(), rename_window.clientHeight()).extend(-0.01f));

   Gui+=remove_window.create().level(1).rect(Rect_C(0, 0, 1.0f, 0.4f)).hide();
   remove_window+=remove_text.create(Vec2  (remove_window.clientWidth()*0.50f, -0.1f), MLTC(u"Are you sure you wish to recycle selected item?", PL,u"Czy jesteś pewien że chcesz przenieść\nwybrany element do kosza?"));
   remove_window+=remove_yes .create(Rect_C(remove_window.clientWidth()*0.25f, -0.25f, 0.28f, 0.07f), MLTC(u"Yes", PL,u"Tak")).func(RemoveDo    , T);
   remove_window+=remove_no  .create(Rect_C(remove_window.clientWidth()*0.75f, -0.25f, 0.28f, 0.07f), MLTC(u"No" , PL,u"Nie")).func(RemoveCancel, T);

   Gui+=overwrite_window.create().level(1).rect(Rect_C(0, 0, 1.0f, 0.4f)).hide();
   overwrite_window+=overwrite_text.create(Vec2  (overwrite_window.clientWidth()*0.50f, -0.1f), MLTC(u"Are you sure you wish to overwrite selected item?", PL,u"Czy jesteś pewien że chcesz nadpisać\nwybrany element?"));
   overwrite_window+=overwrite_yes .create(Rect_C(overwrite_window.clientWidth()*0.25f, -0.25f, 0.28f, 0.07f), MLTC(u"Yes", PL,u"Tak")).func(OverwriteDo    , T);
   overwrite_window+=overwrite_no  .create(Rect_C(overwrite_window.clientWidth()*0.75f, -0.25f, 0.28f, 0.07f), MLTC(u"No" , PL,u"Nie")).func(OverwriteCancel, T);

   up     .size(Vec2(0.16f, 0.06f));
   rename .size(Vec2(0.28f, 0.06f));
   remove .size(Vec2(0.24f, 0.06f));
   explore.size(Vec2(0.26f, 0.06f));
   quick  .size(Vec2(0.06f, 0.06f));
   rect(Rect_C(0, 0, Min(1.3f, D.w2()), Min(1.28f, D.h2())));

   T.ext (ext);
   T.path(path, sub_path  );
   T.io  (load, save, user);
   return T;
}
WindowIO& WindowIO::ext(C Str &ext, C Str &desc)
{
   Memt<Str> exts; Split(exts, ext, '|'); REPA(exts)if(!exts[i].is())exts.remove(i, true); // remove empty extensions and keep order
   if(_dir_operate && !exts.elms())return T; // mode operating on directories requires at least one extension, so if none are specified and we're in that mode, then don't apply the changes
   T._ext=ext; T._ext_desc=desc; _dot_exts.setNum(exts.elms()); FREPAO(_dot_exts)=S+'.'+exts[i];
   return T;
}
WindowIO& WindowIO::path(C Str &path, C Str &sub_path)
{
   T.    _path=Replace(NormalizePath(MakeFullPath(path)), '/', '\\').tailSlash(true);
   T._sub_path=Replace(NormalizePath(         sub_path ), '/', '\\').tailSlash(true); if(StartsPath(_sub_path, ".."))_sub_path.clear();
#if IOS // on iOS we can only write to "SystemPath(SP_APP_DATA)", so make sure that 'T._path' always starts with that path
   Str full=T._path+T._sub_path, root=SystemPath(SP_APP_DATA);
   if(StartsPath(T._path, root)) // T._path already starts with root - "/../documents/xxx" starts with "/../documents"
   {
      // do nothing as paths are already correct
   }else
   if(StartsPath(full, root)) // T._path starts with root but only after combining with T._sub_path - "/../"+"documents/xxx" starts with "/../documents"
   {
      T._path    =                       root .tailSlash(true);
      T._sub_path=SkipStartPath(full, T._path).tailSlash(true);
   }else // path does not start with root at all
   {
      T._path=root.tailSlash(true);
      T._sub_path.clear();
   }
#endif
   if(visible())getList();
   return T;
}
WindowIO& WindowIO::name(C Str &name)
{
   textline.set(name);
   return T;
}
WindowIO& WindowIO::level(Int level)
{
             super::level(level  );
      rename_window.level(level+1);
      remove_window.level(level+1);
   overwrite_window.level(level+1);
   return T;
}
WindowIO& WindowIO::io(void (*load)(C Str &name, Ptr user), void (*save)(C Str &name, Ptr user), Ptr user)
{
   T._load     =load;
   T._save     =save;
   T._func_user=user;
   return T;
}
WindowIO& WindowIO::modeDirSelect()
{
   if(_mode!=WIN_IO_DIR)
   {
     _mode=WIN_IO_DIR;
      list._columns_hidden=true;
      if(list.columns()>=2)
      {
         list.column(0).width+=list.column(1).width;
         list.column(1).hide();
      }
      list.cur_mode=LCM_MOUSE;
      rename.hide();
      remove.hide();
      setRect(); // reset rect because of hidden buttons
   }
   return T;
}
WindowIO& WindowIO::modeDirOperate()
{
   if(!_dir_operate && _dot_exts.elms()) // enable only if there is at least one extension specified
   {
     _dir_operate=true;
   }
   return T;
}
Str WindowIO::final(           )C {return final(textline());}
Str WindowIO::final(C Str &name)C // this will always include 'path', or empty string on error
{
   Str final=NormalizePath(FullPath(name) ? name : path()+subPath()+name);
   return (!path().is() || StartsPath(final, path())) ? final : S;
}
Bool WindowIO::goodExt(C Str &name)C
{
   if(!_dot_exts.elms())return true; // no extensions specified - support all of them
   FREPA(_dot_exts)if(Ends(name, _dot_exts[i]))return true;
   return false;
}
void WindowIO::Ok()
{
   Edit::FileParams fp=final(); if(fp.name.is())
   {
      if(_mode==WIN_IO_DIR)
      {
         if(_load)_load(fp.name, _func_user);
         hide(); // hide at the end, in case it will delete this object
         // !! don't perform any operations on this object afterwards !!
      }else
      {
         // try entering the path if possible
         {
            FileInfo f;
            if(f.getSystem(fp.name))
               if(f.type==FSTD_DRIVE || f.type==FSTD_DIR)
                  if(_dir_operate ? (f.type!=FSTD_DIR || !goodExt(fp.name)) : true)
            {
               path(_path, SkipStartPath(fp.name, _path));
               getList();
               list.scrollTo(0, true);
               textline.clear();
               return;
            }
         }

         // perform operation on file
         {
            Bool hide=true;

            if(ext_mode
            && ((_mode==WIN_IO_SAVE) ? _dot_exts.elms()>=1 : _dot_exts.elms()==1) // for saving add if we have at least 1, for other mode add if we have only 1
            && !goodExt(fp.name)
            && (ext_mode==WIN_IO_EXT_ALWAYS || !GetExt(fp.name).is()))
               fp.name+=_dot_exts[0];
            if(_mode==WIN_IO_LOAD){WindowIORecent(GetPath(fp.name)); if(_load)_load(fp.encode(), _func_user);}
            else                  {hide=!overwriteAsk(fp.name);}

            if(hide)T.hide(); // hide at the end, in case it will delete this object
            // !! don't perform any operations on this object afterwards !!
         }
      }
   }
}
void WindowIO::createDir()
{
   if(!textline().is())Gui.msgBox("Information", "Please first enter the folder name in the textline.");else
   {
      Edit::FileParams fp=final(); if(fp.name.is() && FCreateDirs(fp.name))
      {
         path(_path, SkipStartPath(fp.name, _path));
         getList();
         textline.clear();
      }
   }
}
void WindowIO::renameDo()
{
   if(File *f=list())if(f->type!=FSTD_DRIVE)
   {
     _op_name=path()+subPath()+f->name;
      rename_window  .setTitle(S+MLTC(u"Rename \"", PL,u"Zmień nazwę \"")+f->name+'"').activate();
      rename_textline.set     (f->name).selectExtNot();
   }
}
void WindowIO::removeAsk()
{
   if(File *f=list())if(f->type!=FSTD_DRIVE)
   {
     _op_name=path()+subPath()+f->name;
      remove_window.setTitle(MLT(S+"Move to Recycle Bin \""+f->name+'"',
                             PL,S+u"Przenieś do kosza \""  +f->name+'"')).activate();
      remove_yes.activate();
   }
}
void WindowIO::removeDo()
{
   if(FRecycle(_op_name))
   {
      textline.clear();
      getList();
   }
   remove_window.hide(); disabled(false); list.activate();
}
void WindowIO::removeCancel()
{
   remove_window.hide(); disabled(false); list.activate();
}
Bool WindowIO::overwriteAsk(C Str &name)
{
   if(_save)
   {
     _op_name=name;
      FCreateDirs(GetPath(name));
      if(!FExistSystem(name))
      {
         overwriteDo();
         return true; // don't close the window because we've already closed it inside 'overwriteDo'
      }else
      {
         overwrite_window.setTitle(MLT(S+"Overwrite \""+GetBase(name)+'"',
                                   PL,S+u"Nadpisz \""  +GetBase(name)+'"')).activate();
         overwrite_yes.activate();
         return true; // don't close the window because we're waiting for overwrite decision
      }
   }
   return false; // close the window
}
void WindowIO::overwriteDo()
{
   WindowIORecent(GetPath(_op_name));
   if(_save)_save(_op_name, _func_user);
   overwrite_window.hide(); disabled(false);
   hide(); // hide at the end, in case it will delete this object !! don't perform any operations on this object afterwards !!
}
void WindowIO::overwriteCancel()
{
   overwrite_window.hide(); disabled(false); list.activate();
}
void WindowIO::exploreDo()
{
   if(_path.is() || _sub_path.is())Explore(_path+_sub_path);
}
WindowIO& WindowIO::fullScreen()
{
   FlagDisable(flag, WIN_MOVABLE|WIN_RESIZABLE);
  _border=0.03f;
   rect(D.rect());
   return T;
}
/******************************************************************************/
void WindowIO::setBar()
{
   Str ext=_ext_desc; if(!ext.is())FREPA(_dot_exts){if(ext.is())ext+=", "; ext+=SkipStart(_dot_exts[i], ".");}
#if IOS // on iOS we can only write to "SystemPath(SP_APP_DATA)", so always skip that part
   WIN_IO_PATH_MODE path_mode=((T.path_mode==WIN_IO_PM_FULL) ? WIN_IO_PM_PART : T.path_mode);
#endif
   switch(path_mode)
   {
      case WIN_IO_PM_NONE:
         switch(_mode)
         {
            case WIN_IO_LOAD: setTitle(S+MLTC(u"Load ", PL,u"Wczytaj ")+ext); break;
            case WIN_IO_SAVE: setTitle(S+MLTC(u"Save ", PL,u"Zapisz " )+ext); break;
            case WIN_IO_DIR : setTitle(  MLTC(u"Select Directory", PL,u"Wybierz Katalog")); break;
         }
      break;

      case WIN_IO_PM_PART:
         switch(_mode)
         {
            case WIN_IO_LOAD: setTitle(S+MLTC(u"Load ", PL,u"Wczytaj ")+ext+" - "+subPath()); break;
            case WIN_IO_SAVE: setTitle(S+MLTC(u"Save ", PL,u"Zapisz " )+ext+" - "+subPath()); break;
            case WIN_IO_DIR : if(FullPath(textline()))setTitle(textline());else setTitle(subPath()+textline()); break;
         }
      break;

      case WIN_IO_PM_FULL:
         switch(_mode)
         {
            case WIN_IO_LOAD: setTitle(S+MLTC(u"Load ", PL,u"Wczytaj ")+ext+" - "+SkipStartPath(path()+subPath(), DataPath())); break;
            case WIN_IO_SAVE: setTitle(S+MLTC(u"Save ", PL,u"Zapisz " )+ext+" - "+SkipStartPath(path()+subPath(), DataPath())); break;
            case WIN_IO_DIR : if(FullPath(textline()))setTitle(SkipStartPath(textline(), DataPath()));else setTitle(SkipStartPath(path()+subPath()+textline(), DataPath())); break;
         }
      break;
   }
}
/******************************************************************************/
void WindowIO::getList()
{
   file.clear();
   FileFind ff;
   if(_path.is() || _sub_path.is())ff.find      (_path+_sub_path);
   else                            ff.findDrives(               );
   for(; ff(); )if(!(ff.attrib&FATTRIB_HIDDEN))switch(ff.type)
   {
      case FSTD_DRIVE: if(ff.driveType()==DRIVE_DISK                            )file.New().set(                                                 ff.type,       0, ff.name.tailSlash(false)); break;
      case FSTD_DIR  :                                                           file.New().set((_dir_operate && goodExt(ff.name)) ? FSTD_LINK : ff.type,       0, ff.name                 ); break;
      case FSTD_FILE : if(!_dir_operate && _mode!=WIN_IO_DIR && goodExt(ff.name))file.New().set(                                                 ff.type, ff.size, ff.name                 ); break;
   }
   list.setData(file).lit=-1;
   setBar();
}
/******************************************************************************/
void WindowIO::setFile(C Str &name)
{
   REPA(file)if(Equal(file[i].name, name))
   {
      list.cur=list.absToVis(i);
      if(list.cur>=0)list.scrollTo(list.cur, true, 1);
      return;
   }
}
/******************************************************************************/
void WindowIO::back()
{
   if(_sub_path.last()=='\\' || _sub_path.last()=='/')
   {
     _sub_path.removeLast();
      Str base=_GetBase(_sub_path);
      for(; _sub_path.is() && _sub_path.last()!='\\' && _sub_path.last()!='/'; )_sub_path.removeLast();
      getList();
      setFile(base);
   }
}
void WindowIO::enter(C Str &dir)
{
  _sub_path+=dir;
  _sub_path.tailSlash(true);
   getList();
   list.scrollTo(0, true);
}
/******************************************************************************/
WindowIO& WindowIO::hide()
{
   if(visible())
   {
                super::hide();
         rename_window.hide();
         remove_window.hide();
      overwrite_window.hide();
   }
   return T;
}
WindowIO& WindowIO::show()
{
   if(hidden())
   {
      super::show();
      if(_dot_exts.elms()==1)textline.set(SkipEnd(textline(), _dot_exts[0])); // skip only if we have 1 extension specified
      getList();
   }
   return T;
}
WindowIO& WindowIO::activate()
{
   super::activate();
   return T;
}
WindowIO& WindowIO::save()
{
  _mode=WIN_IO_SAVE;
   setBar();
   return activate();
}
WindowIO& WindowIO::load()
{
   overwrite_window.hide();
  _mode=WIN_IO_LOAD;
   setBar();
   return activate();
}
/******************************************************************************/
Rect WindowIO::sizeLimit()C
{
   Rect r=super::sizeLimit();
   r.min.set(Max(0.70f, ((up     .visible() && up     .is()) ? up     .rect().w() : 0)
                      + ((rename .visible() && rename .is()) ? rename .rect().w() : 0)
                      + ((remove .visible() && remove .is()) ? remove .rect().w() : 0)
                      + ((explore.visible() && explore.is()) ? explore.rect().w() : 0)
                      + ((quick  .visible() && quick  .is()) ? quick  .rect().w() : 0)
                      + 0.03f) + _border*2, 0.48f+_border*2);
   return r;
}
/******************************************************************************/
void WindowIO::setRect()
{
   Rect r(0, -clientHeight(), clientWidth(), 0); r.extendX(-_border).min.y+=_border; r.max.y-=_border*0.5f; // extend top less because there's always the window bar

   // top
   Flt w=r.w(), bw=0; Int n=0;
   if(up     .is() && up     .visible()){bw+=up     .rect().w(); n++;}
   if(rename .is() && rename .visible()){bw+=rename .rect().w(); n++;}
   if(remove .is() && remove .visible()){bw+=remove .rect().w(); n++;}
   if(explore.is() && explore.visible()){bw+=explore.rect().w(); n++;}
   if(quick  .is() && quick  .visible()){bw+=quick  .rect().w(); n++;}
   n--; Flt s=((n>0) ? (w-bw)/n : 0); Vec2 pos=r.lu();
   up     .pos(pos); if(up     .is() && up     .visible())pos.x+=up     .rect().w()+s;
   rename .pos(pos); if(rename .is() && rename .visible())pos.x+=rename .rect().w()+s;
   remove .pos(pos); if(remove .is() && remove .visible())pos.x+=remove .rect().w()+s;
   explore.pos(pos); if(explore.is() && explore.visible())pos.x+=explore.rect().w()+s;
   quick  .pos(pos); if(quick  .is() && quick  .visible())pos.x+=quick  .rect().w()+s;

   // bottom
   create_dir.rect(Rect_LD(r.ld(), 0.38f, 0.06f));
   cancel    .rect(Rect_RD(r.rd(), 0.30f, 0.07f));

   ok        .rect(Rect_RD(cancel.rect().ru()+Vec2(0, 0.02f), 0.30f, 0.07f));
   textline  .rect(Rect(r.min.x, ok.rect().min.y, ok.rect().min.x-_border, ok.rect().max.y));

   // middle
   region.rect(Rect(r.min.x, textline.rect().max.y+0.04f, r.max.x, rename.rect().min.y-0.03f));
   list.columnWidth(0, region.rect().w()-region.slidebarSize()-list.columnWidth(1));
}
WindowIO& WindowIO::rect(C Rect &rect)
{
   if(T.rect()!=rect)
   {
      super::rect(rect);
      setRect();
   }
   return T;
}
/******************************************************************************/
void WindowIO::update(C GuiPC &gpc)
{
   Int list_cur=list.cur;
   super::update(gpc);

   if(Gui.window()==&rename_window) // rename
   {
      if((Kb.k(KB_ENTER) || Kb.k(KB_NPENTER)) && Kb.k.first() && rename_textline().is())
      {
         Kb.eatKey();
         if(FRename(_op_name, final(rename_textline())))
         {
            getList();
            setFile(rename_textline());
         }
         rename_window.hide(); disabled(false); list.activate();
      }else
      if((Kb.k(KB_ESC) || Kb.k(KB_NAV_BACK)) && Kb.k.first() || Ms.bp(2)){Kb.eatKey(); Ms.eat(2); rename_window.hide(); disabled(false); list.activate();}
   }else
   if(Gui.window()==&remove_window) // delete
   {
      if((Kb.k(KB_ESC) || Kb.k(KB_NAV_BACK)) && Kb.k.first() || Ms.bp(2)){Kb.eatKey(); Ms.eat(2); removeCancel();}
   }else
   if(Gui.window()==&overwrite_window) // overwrite
   {
      if((Kb.k(KB_ESC) || Kb.k(KB_NAV_BACK)) && Kb.k.first() || Ms.bp(2)){Kb.eatKey(); Ms.eat(2); overwriteCancel();}
   }

   disabled(rename_window.visible() || remove_window.visible() || overwrite_window.visible());
   if(Gui.window()==this && enabled())
   {
      if((Gui.kb()==&list && Kb.k(KB_BACK)) || (Gui.ms()==&list && (Ms.bp(4) || Ms.bp(1))))
      {
         Kb.eatKey();
         Ms.eat(1);
         Ms.eat(4);
         if(_mode==WIN_IO_DIR)textline.clear();
         back();
      }else
      if(Kb.kf(KB_F2 )                       ){Kb.eatKey(); renameDo ();}else // rename
      if(Kb.kf(KB_DEL) && Gui.kb()!=&textline){Kb.eatKey(); removeAsk();}else // remove
      if(Kb.kf(KB_F4 )                       ){Kb.eatKey(); exploreDo();}     // explore

      // enter/save/load
      if(_mode==WIN_IO_DIR)
      {
         if(Gui.ms()==&list && Ms.bp(0) && !Ms.bd(0) && list.cur>=0)
         {
            if(File *f=list())
            {
               textline.clear();
               enter(f->name);
            }
         }else
         if(Gui.kb()==&list && (Kb.k(KB_ENTER) || Kb.k(KB_NPENTER)) && Kb.k.first() && (list.cur>=0 || list.lit>=0))
         {
            if(File *f=list((list.cur>=0) ? list.cur : list.lit))
            {
               Kb.eatKey();
               textline.clear();
               enter(f->name);
            }
         }else
         if(list.cur>=0)REPA(Touches)if(Touches[i].guiObj()==&list && Touches[i].tapped())
         {
            if(File *f=list())
            {
               textline.clear();
               enter(f->name);
            }
            break;
         }
         if((Kb.k(KB_ENTER) || Kb.k(KB_NPENTER)) && Kb.k.first()){Kb.eatKey(); Ok();} // !! don't perform any operations on this object after 'Ok' !!
      }else
      {
         // change content of TextLine element depending on selected list element
         if(list_cur!=list.cur)
            if(File *f=list())
               if(f->type==(_dir_operate ? FSTD_LINK : FSTD_FILE))textline.set(f->name);

         Bool touch_db=false;

         // enter directory because of the list (enter or double-click)
         if(list.cur>=0)
         {
            REPA(Touches)if(Touches[i].guiObj()==&list && Touches[i].db()){touch_db=true; break;}

            if((Gui.kb()==&list && (Kb.k (KB_ENTER) || Kb.k(KB_NPENTER)) && Kb.k.first())
            || (Gui.ms()==&list &&  Ms.bd(0       )                                     )
            || touch_db)
               if(File *f=list())
                  if(f->type==FSTD_DRIVE || f->type==FSTD_DIR)
            {
               Kb.eatKey();
               Ms.eat(0);
               enter(f->name);
               return;
            }
         }

         if((Kb.k(KB_ENTER) || Kb.k(KB_NPENTER)) && Kb.k.first() || (Gui.ms()==&list && Ms.bd(0) && list.cur>=0) || touch_db)
         {
            Kb.eatKey();
          //Ms.eat(0); don't eat the button on purpose, so Gui.ms() won't be immediately changed
            Ok();
            // !! don't perform any operations on this object after 'Ok' !!
         }
      }
   }
}
/******************************************************************************/
}
/******************************************************************************/
