/******************************************************************************/
extern State StateUpdate;
extern Str   UpdateMessage;
extern State    StateInstall;
extern WindowIO InstallIO;
extern bool     InstallerMode;
/******************************************************************************/
bool UpdateDo();
bool InitUpdate();
void ShutUpdate();
bool UpdateUpdate();
void DrawUpdate();
Str InstallPath();
void SelectInstall(C Str &path, ptr=null);
void ResizeInstall();
bool InitInstall();
void ShutInstall();
bool UpdateInstall();
void DrawInstall();
/******************************************************************************/
