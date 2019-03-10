/******************************************************************************/
/******************************************************************************/
class HeightBrushClass : Window
{
   class AlignWaypoint
   {
      EditWaypoint      *wp;
      EditWaypointPoint *p;

      void set(EditWaypoint &wp, EditWaypointPoint &p);
   };
   Image               noise[2];
   Tabs                mode;
   Memc<Area*>         changed_areas;
   Memc<AlignWaypoint> align_waypoints;
   SyncLock            lock;

   static void ModeChanged(HeightBrushClass &hb);
   
   HeightBrushClass& create();

   static void GetHeight(Cell<Area> &cell, Cursor &cur, int thread_index);
          void getHeight(     Area  &area, Cursor &cur);
   static void UpdateHeight(Cell<Area> &cell, ptr user, int thread_index);
          void updateHeight(     Area  &area);

   void detectWaypoints(C RectI &rect);
   void alignWaypoints();
   void update();

   static void AreaUpdate(Area* &area, ptr user, int thread_index);
   void updateChanged();
};
/******************************************************************************/
/******************************************************************************/
extern HeightBrushClass HeightBrush;
/******************************************************************************/
