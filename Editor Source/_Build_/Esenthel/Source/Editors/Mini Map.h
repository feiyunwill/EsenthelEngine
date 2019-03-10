/******************************************************************************/
extern bool          MiniMapBuilding, MiniMapOk;
extern int           MiniMapAreasLeft;
extern State         StateMiniMap;
/******************************************************************************/
bool InitMiniMap();
void ShutMiniMap();
bool UpdateMiniMap();
void DrawMiniMap();
/******************************************************************************/
