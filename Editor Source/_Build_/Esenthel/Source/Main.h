/******************************************************************************/
extern bool        Initialized;
extern Str         SettingsPath, RunAtExit;
extern Environment DefaultEnvironment;
extern Threads     WorkerThreads, BuilderThreads, BackgroundThreads;
extern Str         CmdLine;
/******************************************************************************/
void ScreenChanged(flt old_width=D.w(), flt old_height=D.h());
bool DisplayReset();
void SetShader();
void Drop(Memc<Str> &names, GuiObj *obj, C Vec2 &screen_pos);
bool SaveChanges(void (*after_save_close)(bool all_saved, ptr user), ptr user=null);
void SetExit(bool all_saved=true, ptr=null);
void Quit();
void Resumed();
void ReceiveData(cptr data, int size, ptr hwnd_sender);
void SetTitle();
void SetKbExclusive();
void SetProjectState();
Rect EditRect(bool modes=true);
Environment& CurrentEnvironment();
void InitPre();
bool Init();
void Shut();
bool Update();
void Draw  ();
/******************************************************************************/
