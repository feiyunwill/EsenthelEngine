/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Net{
/******************************************************************************/
Cache<World> Worlds("World");
/******************************************************************************/
Area::Area(C VecI2 &xy, Ptr grid_user)
{
   T._xy=xy;
}
/******************************************************************************/
World::World()
{
   grid.user(this);
  _id.zero();
      _area_size=32;
  _neighbor_dist=64; _neighbor_dist2=Sqr(_neighbor_dist);
}
/******************************************************************************/
World& World::areaSize(Flt size)
{
  _area_size=Max(size, EPS);
   return T;
}
World& World::neighborDist(Flt dist)
{
  _neighbor_dist =Max(dist, 0);
  _neighbor_dist2=Sqr(_neighbor_dist);
   return T;
}
/******************************************************************************/
Bool World::load(C Str &name)
{
   if(!DecodeFileName(name, _id))_id.zero();
   T._name     =name;
   T._data_path=name; _data_path.tailSlash(true);
   Game::WorldSettings settings; if(settings.load(dataPath()+"Settings"))areaSize(settings.areaSize());
   return true;
}
/******************************************************************************/
}}
/******************************************************************************/
