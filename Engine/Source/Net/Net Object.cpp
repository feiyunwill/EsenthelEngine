/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Net{
/******************************************************************************/
#define CC4_NOBJ CC4('N','O','B','J')

#define NEIGHBOR_UPDATE_DISTANCE 3.0 // travel distance after which neighbors are updated
/******************************************************************************/
static Bool AreNeighbors(C Vec &a, C Vec &b, World *world)
{
   return Dist2(a.xz(), b.xz())<=world->_neighbor_dist2;
}
/******************************************************************************/
Obj::Obj()
{
  _pos.zero(); _old_pos.zero(); _area=null; _world=null;
}
/******************************************************************************/
// GET / SET
/******************************************************************************/
Obj& Obj::pos(C Vec &pos)
{
   T._pos=pos;
   if(inGame())
   {
      VecI2 xy=_world->worldToArea(pos);
      if(_area->xy()!=xy)putToArea(*_world->grid.get(xy).data());
      if(Dist2(_old_pos.xz(), pos.xz())>=Sqr(NEIGHBOR_UPDATE_DISTANCE)) // after moving NEIGHBOR_UPDATE_DISTANCE meters
      {
        _old_pos=pos;
         updateNeighbors();
      }
   }else
   {
     _old_pos=pos;
   }
   return T;
}
/******************************************************************************/
void Obj::setWorld(World *world, C Vec &pos)
{
   leaveGame();
   T._world=world;
   T.pos(pos);
}
/******************************************************************************/
// OPERATIONS
/******************************************************************************/
void Obj::enterGame()
{
   if(!inGame() && _world)
   {
      putToArea(*_world->grid.get(_world->worldToArea(pos())).data());
      updateNeighbors();
   }
}
void Obj::leaveGame()
{
   removeFromArea ();
   removeNeighbors();
}
/******************************************************************************/
void Obj::removeFromArea()
{
   if(_area)
   {
     _area->_objs.exclude(this);
     _area=null;
   }
}
void Obj::putToArea(Area &area)
{
   removeFromArea();
   T._area=&area;
   T._area->_objs.include(this);
}
/******************************************************************************/
static void Connect(Obj &a, Obj &b)
{
   if(&a!=&b)
   {
      REP(a.neighbors())if(&a.neighbor(i).obj()==&b)return; // already are neighbors

      // first add to containers
      a._neighbors.New()._obj=&b;
      b._neighbors.New()._obj=&a;

      // then run callbacks
      a.connected(b);
      b.connected(a);
   }
}
static void Disconnect(Obj &a, Obj &b)
{
   if(&a!=&b)REP(a.neighbors())if(&a.neighbor(i).obj()==&b)
   {
      // first remove from containers
                                                    a._neighbors.remove(i);
      REP(b.neighbors())if(&b.neighbor(i).obj()==&a)b._neighbors.remove(i);

      // then run callbacks
      a.disconnected(b);
      b.disconnected(a);

      break;
   }
}
static void DetectNeighbors(Cell<Area> &cell, Obj &obj)
{
   Area &area=cell();
   REP(  area.objs())if(InRange(i, area.objs())) // InRange in case 'connect' will somehow modify the 'area.objs'
   {
      Obj &test=area.obj(i);
      if(AreNeighbors(test.pos(), obj.pos(), obj.world()))Connect(obj, test);
   }
}
void Obj::updateNeighbors()
{
   // update existing neighbors
   REP(neighbors())
      if(InRange(i, neighbors())) // InRange in case 'disconnect' will somehow modify the 'neighbors' container
         if(!AreNeighbors(pos(), neighbor(i).obj().pos(), world()))
            Disconnect(T, neighbor(i).obj());

   // add new neighbors
   if(world())world()->grid.func(world()->worldToArea(Rect_C(pos().xz(), world()->neighborDist())), DetectNeighbors, T);
}
void Obj::removeNeighbors()
{
   REP(neighbors())
      if(InRange(i, neighbors())) // InRange in case 'disconnect' will somehow modify the 'neighbors' container
         Disconnect(T, neighbor(i).obj());
}
/******************************************************************************/
// NETWORK
/******************************************************************************/
void Obj::compress(File &f, C StrLibrary *worlds)
{
   if(worlds)worlds->putStr(f, Worlds.name(world()));else f.putStr(Worlds.name(world()));
   f<<pos();
}
void Obj::decompress(File &f, C StrLibrary *worlds)
{
   if(worlds)_world=Worlds(worlds->getStr(f));else _world=Worlds(f.getStr());
   f>>_pos;
}
/******************************************************************************/
// IO
/******************************************************************************/
Bool Obj::save(File &f)
{
   f.cmpUIntV(0); // version
   f.putAsset(Worlds.name(world())); // operate on Str instead of ID in case user uses a custom world name
   f<<pos();
   return f.ok();
}
Bool Obj::load(File &f)
{
   switch(f.decUIntV()) // version
   {
      case 0:
      {
        _world=Worlds(f.getAsset()); // operate on Str instead of ID in case user uses a custom world name
         f>>_pos;
         if(f.ok())return true;
      }break;
   }
   return false;
}
/******************************************************************************/
}}
/******************************************************************************/
