/******************************************************************************/
namespace Net{
/******************************************************************************/
struct Area // Net Area
{
   // get
 C VecI2& xy()C {return _xy;} // get area coordinates

   Int  objs(     ) {return  _objs.elms();} // get number of objects in the area
   Obj& obj (Int i) {return *_objs[i]    ;} // get i-th      object  in the area

   virtual ~Area(                          ) {} // force virtual class to enable memory container auto-casting when extending this class with another virtual class
            Area(C VecI2 &xy, Ptr grid_user);

#if !EE_PRIVATE
private:
#endif
   VecI2      _xy;
   Memc<Obj*> _objs;
};
/******************************************************************************/
struct World // Net World
{
   Grid<Area> grid;

   // get
 C UID& id      ()C {return _id;}
 C Str& name    ()C {return _name;}
 C Str& dataPath()C {return _data_path;}

   Flt     areaSize()C {return     _area_size;}   World&     areaSize(Flt size); // get/set area         size (in meters), default=32
   Flt neighborDist()C {return _neighbor_dist;}   World& neighborDist(Flt dist); // get/set neighbor distance (in meters), default=64 (neighbor distance is the distance between objects at which they are considered to be neighbors, and exchange information between each other, for example characters are visible to each other only if their distance is less than neighbor distance)

   VecI2 worldToArea(C Vec2 &pos )C {return                               Floor(pos/areaSize());}
   VecI2 worldToArea(C Vec  &pos )C {return                              worldToArea(pos .xz());}
   RectI worldToArea(C Rect &rect)C {return RectI(worldToArea(rect.min), worldToArea(rect.max));}
   RectI worldToArea(C Box  &box )C {return RectI(worldToArea(box .min), worldToArea(box .max));}

   Game::Waypoint* findWaypoint(C Str &name)C {return name.is   () ? Game::Waypoints.get(dataPath()+"Waypoint/"+name              ) : null;} // find waypoint in this world, null on fail
   Game::Waypoint* findWaypoint(C UID &id  )C {return id  .valid() ? Game::Waypoints.get(dataPath()+"Waypoint/"+EncodeFileName(id)) : null;} // find waypoint in this world, null on fail
   Game::Waypoint*  getWaypoint(C Str &name)C {return name.is   () ? Game::Waypoints    (dataPath()+"Waypoint/"+name              ) : null;} // get  waypoint in this world, Exit on fail
   Game::Waypoint*  getWaypoint(C UID &id  )C {return id  .valid() ? Game::Waypoints    (dataPath()+"Waypoint/"+EncodeFileName(id)) : null;} // get  waypoint in this world, Exit on fail

   // io
   Bool load(C Str &name);

   World();

#if !EE_PRIVATE
private:
#endif
   Flt _area_size, _neighbor_dist, _neighbor_dist2;
   Str _name, _data_path;
   UID _id;
};
/******************************************************************************/
extern Cache<World> Worlds;
/******************************************************************************/
} // namespace
/******************************************************************************/
