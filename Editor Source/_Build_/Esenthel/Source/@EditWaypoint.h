/******************************************************************************/
/******************************************************************************/
class EditWaypoint : Memc<EditWaypointPoint>
{
   static int                FindI(C Memc<EditWaypointPoint> &waypoints, C UID &waypoint_id);
   static EditWaypointPoint* Find (  Memc<EditWaypointPoint> &waypoints, C UID &waypoint_id);

   bool                    removed;
   Game::Waypoint::LOOP_MODE loop_mode;
   Str                     name;
   TimeStamp               removed_time, loop_mode_time, name_time;

   void del();

   int                findI(C UID &waypoint_id)C;
   EditWaypointPoint* find (C UID &waypoint_id); 
 C EditWaypointPoint* find (C UID &waypoint_id)C;

   int liveIndex(C UID &waypoint_id);

   bool newer(C EditWaypoint &src)C;
   bool equal(C EditWaypoint &src)C;
   bool getRect(Rect &rect)C;
   bool  hasExisting()C;              
   UID firstExisting()C;              
   UID existingNear(C UID &point_id)C;
   Vec pos()C;
   int existingPoints()C;

   EditWaypoint& setRemoved (bool                    removed  );  
   EditWaypoint& setLoopMode(Game::Waypoint::LOOP_MODE loop_mode);
   EditWaypoint& setName    (C Str                  &name     );

   bool sync(C EditWaypoint &src);
   void undo(C EditWaypoint &src);
   int move(C UID &point_id, int index);
   EditWaypoint& reverse();
   EditWaypoint& rotateLeft();
   EditWaypoint& rotateRight();

   void create(C Game::Waypoint &src, C Str &name);
   bool copyTo(Game::Waypoint &w);

   // draw
   void draw(C Color &color=WHITE, int edge_smooth_steps=1);
   void drawText(C UID &sel, C UID &lit);

   // io
   bool save(File &f)C;
   bool load(File &f);
   bool load(C Str &name);

public:
   EditWaypoint();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
