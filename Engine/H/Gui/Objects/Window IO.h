/******************************************************************************/
enum WIN_IO_MODE : Byte // WindowIO mode
{
   WIN_IO_LOAD, // load file
   WIN_IO_SAVE, // save file
   WIN_IO_DIR , // directory select
};
enum WIN_IO_PATH_MODE : Byte // WindowIO title bar path display mode
{
   WIN_IO_PM_NONE, // none  , path                   will not be included in the title bar
   WIN_IO_PM_PART, // partly, only sub-path          will     be included in the title bar
   WIN_IO_PM_FULL, // fully , full path and sub-path will     be included in the title bar
};
enum WIN_IO_EXT_MODE : Byte // WindowIO extension append mode
{
   WIN_IO_EXT_NEVER , // never  append extension
   WIN_IO_EXT_EMPTY , //        append extension only when not specified
   WIN_IO_EXT_ALWAYS, // always append extension
};
/******************************************************************************/
const_mem_addr STRUCT(QuickPathSelector , ComboBox)
//{
   ComboBox& create();
   ComboBox& create(C Rect &rect) {return create().rect(rect);} // create and set rectangle

   virtual Str  getPath(           ) {return S;} // get current path, extend this function to return current path in your custom file browser
   virtual void setPath(C Str &path) {         } // set current path, extend this function to set    current path in your custom file browser
#if EE_PRIVATE
   void setData(); // call this to update path selector paths from the global variables
#endif

#if !EE_PRIVATE
private:
#endif
   void (*_func)(Ptr user);
};
/******************************************************************************/
const_mem_addr STRUCT(WindowIO , ClosableWindow) // Gui Window Input Output !! must be stored in constant memory address !!
//{
   WIN_IO_PATH_MODE path_mode; // path     display mode, default=WIN_IO_PM_FULL
   WIN_IO_EXT_MODE   ext_mode; // extension append mode, default=WIN_IO_EXT_ALWAYS

   Button   ok        , // ok
            cancel    , // cancel
            create_dir, // create directory
            up        , // up folder
            rename    , // rename
            remove    , // delete
            explore   ; // explore
   TextLine textline  ; // text line
   Region   region    ; // region

   Window   rename_window  ; // rename window
   TextLine rename_textline; // rename text line

   Window remove_window; // delete window
   Text   remove_text  ; // delete text
   Button remove_yes   , // delete yes
          remove_no    ; // delete no

   Window overwrite_window; // overwrite window
   Text   overwrite_text  ; // overwrite text
   Button overwrite_yes   , // overwrite yes
          overwrite_no    ; // overwrite no

   // manage
            WindowIO& del   (                                                                                                                                                       ); // delete
            WindowIO& create(C Str &ext=S, C Str &path=S, C Str &sub_path=S, void (*load)(C Str &name, Ptr   user)=null, void (*save)(C Str &name, Ptr   user)=null, Ptr   user=null); // create
   T1(TYPE) WindowIO& create(C Str &ext  , C Str &path  , C Str &sub_path  , void (*load)(C Str &name, TYPE *user)     , void (*save)(C Str &name, TYPE *user)     , TYPE *user     ) {return create(ext, path, sub_path).io(load, save, user);}
   T1(TYPE) WindowIO& create(C Str &ext  , C Str &path  , C Str &sub_path  , void (*load)(C Str &name, TYPE &user)     , void (*save)(C Str &name, TYPE &user)     , TYPE &user     ) {return create(ext, path, sub_path).io(load, save, user);}

   // set / get
   WindowIO& ext (C Str &ext , C Str &desc    =S);   C Str& ext    ()C {return      _ext;} // set/get supported extensions, 'ext'=list of supported extensions separated with '|' symbol (null for all kinds of extensions), 'desc'=description of supported files types (if specified then it will be used in the window title bar instead of the 1st extension)
   WindowIO& path(C Str &path, C Str &sub_path=S);   C Str& path   ()C {return     _path;} // set/get io path's, 'path'=main path, 'sub_path'=sub-path
                                                     C Str& subPath()C {return _sub_path;} //     get sub-path
   WindowIO& name(C Str &name                   );                                         // set     default file name, this is equal to setting the textline value to 'name'

            WindowIO& modeDirSelect (                                                                                                       ); // set a special mode which is used for directory selection
            WindowIO& modeDirOperate(                                                                                                       ); // set a special mode which is used for saving/loading directories with a specified extension (instead of files), in order to activate this mode - extension must be set
            WindowIO& level         (Int level                                                                                              ); // set window levels
            WindowIO& io            (void (*load)(C Str &name, Ptr   user)=null, void (*save)(C Str &name, Ptr   user)=null, Ptr   user=null); // set io functions
   T1(TYPE) WindowIO& io            (void (*load)(C Str &name, TYPE *user)     , void (*save)(C Str &name, TYPE *user)     , TYPE *user     ) {return io((void(*)(C Str&, Ptr))load, (void(*)(C Str&, Ptr))save,  user);}
   T1(TYPE) WindowIO& io            (void (*load)(C Str &name, TYPE &user)     , void (*save)(C Str &name, TYPE &user)     , TYPE &user     ) {return io((void(*)(C Str&, Ptr))load, (void(*)(C Str&, Ptr))save, &user);}

   // operations
   virtual WindowIO& hide    (); // hide
   virtual WindowIO& show    (); // show
   virtual WindowIO& activate(); // activate
   virtual WindowIO& save    (); // activate to save
   virtual WindowIO& load    (); // activate to load

#if EE_PRIVATE
   void zero           (           );
   void setBar         (           );
   void setRect        (           );
   void getList        (           );
   void setFile        (C Str &name);
   void back           (           );
   void enter          (C Str &dir );
   Str  final          (C Str &name)C;
   Str  final          (           )C;
   void Ok             (           );
   void createDir      (           );
   void renameDo       (           );
   void removeAsk      (           );
   void removeDo       (           );
   void removeCancel   (           );
   Bool overwriteAsk   (C Str &name);
   void overwriteDo    (           );
   void overwriteCancel(           );
   void exploreDo      (           );
   Bool goodExt        (C Str &name)C;
   WindowIO& fullScreen(           ); // set full screen mode
#endif

   virtual Rect      sizeLimit(             )C;                                           // set     allowed size limits for the Window rectangle, you can override this method and return custom values, they will be used by 'rect' method
   virtual WindowIO& rect     (C Rect  &rect) ;   C Rect& rect()C {return super::rect();} // set/get rectangle
   virtual void      update   (C GuiPC &gpc ) ;

  ~WindowIO() {del();}
   WindowIO();

#if !EE_PRIVATE
private:
#endif
   struct File // WindowIO File
   {
      UInt type; // file type (this is FSTD_TYPE but keep as UInt so 'List.setElmType' which operates on UInt can be used)
      Long size; // file size
      Str  name; // file name

   #if EE_PRIVATE
      void set(FSTD_TYPE type, Long size, C Str &name) {T.type=type; T.size=size; T.name=name;}
   #endif
   };
   const_mem_addr STRUCT(QPS , QuickPathSelector)
   //{
      WindowIO &win_io;
      virtual Str  getPath(           );
      virtual void setPath(C Str &path);
      QPS(WindowIO &win_io);
   };

   WIN_IO_MODE _mode;
   Bool        _dir_operate;
   Flt         _border;
   Ptr         _func_user;
   void      (*_load)(C Str &name, Ptr user),
             (*_save)(C Str &name, Ptr user);
   Str         _ext, _ext_desc, _path, _sub_path, _op_name;
   Mems<Str >  _dot_exts;
   Memc<File>   file;
   List<File>   list;
   MenuBar      menu;
   QPS          quick;

   NO_COPY_CONSTRUCTOR(WindowIO);
};
/******************************************************************************/
extern Memc<Str> WindowIOFavorites, WindowIORecents;
/******************************************************************************/
