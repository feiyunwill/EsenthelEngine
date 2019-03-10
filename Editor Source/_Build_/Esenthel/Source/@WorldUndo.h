/******************************************************************************/
/******************************************************************************/
class WorldUndo : Edit::Undo<WorldChange>
{
   static cchar8 * TerrainType ,
                 *     ObjType     ,
                 *  NewObjType  ,
                 *WaypointType;

   SyncLock lock;

   WorldUndo();

   void del();

   WorldChange* set(cptr type, bool force_create=false); // !! this may get called on multiple threads when processing different areas !!
   void setTerrain(C Area &area); // !! this may get called on multiple threads when processing different areas !!
   void set(C ObjData &obj, cptr type, bool as_new);
   void set(C EditWaypoint &waypoint, cptr type=WaypointType);
   void set(C Lake &lake);
   void set(C River &river);
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
