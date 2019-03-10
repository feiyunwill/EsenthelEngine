/******************************************************************************/
/******************************************************************************/
class WorldData
{
   UID            elm_id;
   Elm           *elm;
   Grid<Area>     grid;
   Str            edit_path, game_path, edit_area_path, game_area_path, edit_waypoint_path, game_waypoint_path, lake_path, river_path;
   flt            area_size;
   int            hm_res;
   WorldVer      *ver;
   Memx<Obj>      objs;
   PathWorld      path_world;
   PathSettings   path_settings;
   EnvironmentPtr environment;

   WorldData();

   flt areaSize()C;
   int   hmRes ()C;

   Vec2   areaToWorld(C VecI2 &xz  )C; // convert Area  Coordinates to World Position
   VecI2 worldToArea (C Vec2  &xz  )C; // convert World Position    to Area  Coordinates
   VecI2 worldToArea (C Vec   &pos )C; // convert World Position    to Area  Coordinates
   RectI worldToArea (C Rect  &rect)C; // convert World Position    to Area  Coordinates
   RectI worldToArea (C Box   &box )C; // convert World Position    to Area  Coordinates

   Area*  getArea      (C VecI2 &area_xy);
   Area* findArea      (C VecI2 &area_xy);
   Area* findAreaLoaded(C VecI2 &area_xy);

   void load();
   void flush();
   void unload();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
