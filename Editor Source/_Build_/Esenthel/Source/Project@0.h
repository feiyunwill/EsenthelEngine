/******************************************************************************/
extern ThreadSafeMap<UID, TextureInfo> TexInfos;
extern State StateProject;
/******************************************************************************/
int CompareProjPath(C UID &a, C UID &b);
void HideProject();
bool InitProject();
void ShutProject();
Elm* GuiObjToElm(GuiObj *go);
bool UpdateProject();
void DrawProject();
/******************************************************************************/
