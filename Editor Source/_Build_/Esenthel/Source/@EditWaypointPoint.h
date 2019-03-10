/******************************************************************************/
/******************************************************************************/
class EditWaypointPoint
{
   bool      removed;
   UID       id;
   Vec       pos;
   TimeStamp removed_time, pos_time, order_time;

   void setPos    (C Vec &pos  );
   void setRemoved(bool removed);

   bool newer(C EditWaypointPoint &src)C;
   bool equal(C EditWaypointPoint &src)C;
   bool sync(C EditWaypointPoint &src);
   bool undo(C EditWaypointPoint &src);

   // io
   bool save(File &f)C;
   bool load(File &f);

public:
   EditWaypointPoint();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
