/******************************************************************************/
extern Memc<IDReplace> ReplaceIDs;
extern State           IDReplaceState;
/******************************************************************************/
C IDReplace*    ReplaceID(C UID &src, Memc<IDReplace> &replace=ReplaceIDs);
bool ThreadIDReplace(Thread &thread);
bool InitIDReplace();
void ShutIDReplace();
bool UpdateIDReplace();
void DrawIDReplace();
/******************************************************************************/
