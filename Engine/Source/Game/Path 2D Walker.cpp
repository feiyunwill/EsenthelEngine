/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Game{
/******************************************************************************/
Path2DWalker& Path2DWalker::clear()
{
   points.clear();
   return T;
}
/******************************************************************************/
static void SetWalkable(AreaPath2D &path, Int ofs_x, Int ofs_y)
{
   REPD(y, path._map.h())
   REPD(x, path._map.w())World._path_find.pixelFlag(ofs_x+x, ofs_y+y, path.walkable(x, y) ? PFP_WALKABLE : 0);
}
static void SetNonwalkable(Int ofs_x, Int ofs_y)
{
   REP(World.settings().path2DRes())
   {
      World._path_find.pixelFlag(ofs_x+i, ofs_y                               , 0);
      World._path_find.pixelFlag(ofs_x+i, ofs_y+World.settings().path2DRes()-1, 0);
      World._path_find.pixelFlag(ofs_x                               , ofs_y+i, 0);
      World._path_find.pixelFlag(ofs_x+World.settings().path2DRes()-1, ofs_y+i, 0);
   }
}
/******************************************************************************/
static Bool PathWalkerFind(Int start_node, Int target_node, C VecI2 &start_xy, C VecI2 &target_xy, Memc<Vec2> &points)
{
   PathNode & start_pn=World._path_node[ start_node],
            &target_pn=World._path_node[target_node];

   if(start_pn.type==PN_AREA && target_pn.type==PN_AREA)
   if(Cell<Area> * start_g=                                                   World._grid.find( start_pn.area.xy))
   if(Cell<Area> *target_g=((start_pn.area.xy==target_pn.area.xy) ? start_g : World._grid.find(target_pn.area.xy)))
   {
      if(Abs(start_g->x()-target_g->x())>1
      || Abs(start_g->y()-target_g->y())>1)return false; // too far away

      AreaPath2D & start_path=* start_g->data()->_data->path2D(),
                 &target_path=*target_g->data()->_data->path2D();

      // valid range
      World._path_find.border(0, 0, (start_g->x()==target_g->x()) ? World.settings().path2DRes() : World.settings().path2DRes()*2,
                                    (start_g->y()==target_g->y()) ? World.settings().path2DRes() : World.settings().path2DRes()*2);

      // start offset
      Int start_ofs_x=((start_g->x()>target_g->x()) ? World.settings().path2DRes() : 0),
          start_ofs_y=((start_g->y()>target_g->y()) ? World.settings().path2DRes() : 0);

      // target offset
      Int target_ofs_x=((target_g->x()>start_g->x()) ? World.settings().path2DRes() : 0),
          target_ofs_y=((target_g->y()>start_g->y()) ? World.settings().path2DRes() : 0);

      // set path map
      {
         // set path find flag to walkable
                               SetWalkable( start_path,  start_ofs_x,  start_ofs_y);
         if(start_g!=target_g){SetWalkable(target_path, target_ofs_x, target_ofs_y);
            if(start_g->x()!=target_g->x()
            && start_g->y()!=target_g->y()) // diagonal
            {
               Cell<Area> *cell;
               AreaPath2D *path;
               
               path=null; if(cell=World._grid.find(target_g->xy()))if(cell->data()->loaded() && cell->data()->_data)path=cell->data()->_data->path2D(); if(path)SetWalkable(*path, target_ofs_x,  start_ofs_y);else SetNonwalkable(target_ofs_x,  start_ofs_y);
               path=null; if(cell=World._grid.find( start_g->xy()))if(cell->data()->loaded() && cell->data()->_data)path=cell->data()->_data->path2D(); if(path)SetWalkable(*path,  start_ofs_x, target_ofs_y);else SetNonwalkable( start_ofs_x, target_ofs_y);
            }
         }

         // include path find target flag
         if(InRange(target_xy.x, World.settings().path2DRes())
         && InRange(target_xy.y, World.settings().path2DRes()))World._path_find.pixel(target_ofs_x+target_xy.x, target_ofs_y+target_xy.y).flag|=PFP_TARGET;else // we have precise target position
         {
            REPD(y, target_path._map.h())
            REPD(x, target_path._map.w())if(target_path._map.pixB(x, y)==target_pn.area.index)World._path_find.pixel(target_ofs_x+x, target_ofs_y+y).flag|=PFP_TARGET; // target is the whole node
         }
      }

      // find path
      Memc<VecI2> path;
      if(World._path_find.find(&VecI2(start_xy.x+start_ofs_x, start_xy.y+start_ofs_y), null, path, -1, true, true))
      {
         // convert local VecI2 to world Vec2
         Vec2 mul=World.areaSize()/World.settings().path2DRes(),
              add=Vec2(start_g->xy())*World.areaSize() + Vec2(0.5f-start_ofs_x, 0.5f-start_ofs_y)*mul;
         FREPA(path)points.add(Vec2(path[i])*mul + add);
         return true;
      }
   }
   return false;
}
/******************************************************************************/
static inline Bool PathWalkerFind(Int start_node, Int target_node, C VecI2 &start_xy, Memc<Vec2> &points)
{
   return PathWalkerFind(start_node, target_node, start_xy, VecI2(-1), points);
}
/******************************************************************************/
static Bool PathWalkerFindStraight(C Vec &start, C Vec &target, Memc<Vec2> &points, Flt allowed_range=World.areaSize())
{
   if(Dist2(start.xz(), target.xz())<=Sqr(allowed_range))
   {
      AreaPath2D *path       =null;
      VecI2       area_cur   =SIGN_BIT,
                  area_start =World.worldToArea(start);
      Vec2        path_start =(start .xz()/World.areaSize()-Vec2(area_start))*World.settings().path2DRes(),
                  path_target=(target.xz()/World.areaSize()-Vec2(area_start))*World.settings().path2DRes();

      for(PixelWalker ew(path_start, path_target); ew.active(); ew.step())
      {
         VecI2 path_pos =ew.pos(),
               area_new =area_start+VecI2(DivFloor(path_pos.x, World.settings().path2DRes()), DivFloor(path_pos.y, World.settings().path2DRes()));
         if(   area_new!=area_cur)
         {
            Area::Data *data=World._grid.get(area_cur=area_new).data()->_data; if(!data)return false;
                        path=data->path2D();                                   if(!path)return false;
         }
         if(!path->walkable(Mod(path_pos.x, World.settings().path2DRes()), Mod(path_pos.y, World.settings().path2DRes())))return false;
      }
      points.add(target.xz());
      return true;
   }
   return false;
}
/******************************************************************************/
Bool Path2DWalker::find(C Vec &start, C Vec &target, C Vec *alternate_start)
{
   clear();
   T.target=target;

   if(Dist2(start.xz(), target.xz())<=Sqr(0.1f))return true; // we're already at the target

   Int   start_node, target_node;
   VecI2 start_xy  , target_xy  ;

                                      start_node=World.pathGetNode(           start, start_xy);
   if(start_node<0 && alternate_start)start_node=World.pathGetNode(*alternate_start, start_xy); // if current start position is invalid try using last known position (if given)
   if(start_node>=0)
   {
         target_node =World.pathGetNode(target, target_xy);
      if(target_node>=0)
      {
         if(PathWalkerFindStraight(start, target, points))return true; // try first going straight

         if(start_node==target_node && start_xy==target_xy)return true; // we're already at the target

         Memc<UInt> node;
         if(World.pathFind(start_node, target_node, node)) // find in nodes
         {
            if((node.elms()<=1) // start and target are located in the same or adjacent nodes
               ? PathWalkerFind(start_node, target_node, start_xy, target_xy, points)
               : PathWalkerFind(start_node, node.last(), start_xy,            points))
            {
               return true;
            }
         }
      }
   }

   return false;
}
/******************************************************************************/
}}
/******************************************************************************/
