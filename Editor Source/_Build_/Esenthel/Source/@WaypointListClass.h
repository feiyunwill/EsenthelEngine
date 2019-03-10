/******************************************************************************/
/******************************************************************************/
class WaypointListClass : ClosableWindow
{
   class Elm
   {
      EditWaypoint *waypoint;

public:
   Elm();
   };
   bool     _changed;
   Button    refresh;
   TextLine  filter;
   Button    show_removed;
   Region    region;
   List<Elm> list;
   Memx<Elm> data;

   static void FilterChanged(WaypointListClass &ol);
   static void          Hide(WaypointListClass &ol);

   static Str WaypointName  (C Elm &elm);
   static Str WaypointPoints(C Elm &elm);
   static Str WaypointPosX  (C Elm &elm);
   static Str WaypointPosY  (C Elm &elm);
   static Str WaypointPosZ  (C Elm &elm);

   void setChanged();         
   WaypointListClass&create();

   void setCur();
   void setVisible();
   void set();

   virtual Rect sizeLimit()C override;
           C Rect& rect()C;                   
   virtual Window& rect(C Rect &rect)override;
   virtual void update(C GuiPC &gpc)override;
   void removed(EditWaypoint &waypoint);

public:
   WaypointListClass();
};
/******************************************************************************/
/******************************************************************************/
extern WaypointListClass WaypointList;
/******************************************************************************/
