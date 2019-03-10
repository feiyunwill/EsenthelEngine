/******************************************************************************/
#include "stdafx.h"
namespace EE{
namespace Game{
/******************************************************************************/
static void SetBasePathNodes(Area &area)
{
   if(Area::Data *data=area.data())
   {
      if(!data->path2D())data->_path_node_offset=-1;else
      {
         AreaPath2D &path=*data->path2D();
         data->_path_node_offset=World._path_node.elms();
         if  (path._changed)path.group();
         FREP(path._groups )
         {
            PathNode &pn=World._path_node.New();
            pn.node_index=data->_path_node_offset+i;

            pn.type    = PN_AREA;
            pn.nghb_num= 0;
            pn.nghb_ofs= 0;
            pn.parent  =-1;                         // no parent
            pn.sibling = data->_path_node_offset+i; // self

            pn.area.xy   =area.xz();
            pn.area.index=i;
         }
      }
   }
}
static VecI2 Offsets[]=
{
   VecI2(-1, 1),
   VecI2( 0, 1),
   VecI2( 1, 1),

   VecI2(-1, 0),
 //VecI2( 0, 0),
   VecI2( 1, 0),

   VecI2(-1,-1),
   VecI2( 0,-1),
   VecI2( 1,-1),
};
struct SetNeighborsHelper
{
   PathNodeNeighbor neighbor [256][256]; // [group][neighbor]
   Int              neighbors[256]     ; // [group]
   Area::Data      *data, *neighbor_data, *data_array[3][3]; // [y][x] orientation

   void addNeighbor(Int group, Int neighbor_index, Int cost)
   {
      // check if it's already written
      REP(neighbors[group]) // iterate through all neighbors of a node
      {
         if(neighbor[group][i].index==neighbor_index) // if i-th neighbor is 'neighbor_index'
         {
            MIN(neighbor[group][i].cost, cost); // decrease the travel cost
            return; // neighbor already exists so return
         }
      }

      // add new neighbor
      neighbor[group][neighbors[group]++].set(neighbor_index, cost);
   }

   void testLeft(Int y)
   {
      Byte group =data->path2D()->_map.pixB(0, y);
      if(  group!=0xFF)
      {
         Byte group_b          =         data->path2D()->_map.pixB(                             0, y-1),
              group_f          =         data->path2D()->_map.pixB(                             0, y+1),
              neighbor_group_lb=neighbor_data->path2D()->_map.pixB(World.settings().path2DRes()-1, y-1),
              neighbor_group_l =neighbor_data->path2D()->_map.pixB(World.settings().path2DRes()-1, y  ),
              neighbor_group_lf=neighbor_data->path2D()->_map.pixB(World.settings().path2DRes()-1, y+1);

         if(neighbor_group_l !=0xFF                                             )addNeighbor(group, neighbor_data->_path_node_offset+neighbor_group_l , 5);
         if(neighbor_group_lb!=0xFF && (neighbor_group_l!=0xFF || group_b!=0xFF))addNeighbor(group, neighbor_data->_path_node_offset+neighbor_group_lb, 7);
         if(neighbor_group_lf!=0xFF && (neighbor_group_l!=0xFF || group_f!=0xFF))addNeighbor(group, neighbor_data->_path_node_offset+neighbor_group_lf, 7);
      }
   }
   void testRight(Int y)
   {
      Byte group =data->path2D()->_map.pixB(World.settings().path2DRes()-1, y);
      if(  group!=0xFF)
      {
         Byte group_b          =         data->path2D()->_map.pixB(World.settings().path2DRes()-1, y-1),
              group_f          =         data->path2D()->_map.pixB(World.settings().path2DRes()-1, y+1),
              neighbor_group_rb=neighbor_data->path2D()->_map.pixB(                             0, y-1),
              neighbor_group_r =neighbor_data->path2D()->_map.pixB(                             0, y  ),
              neighbor_group_rf=neighbor_data->path2D()->_map.pixB(                             0, y+1);

         if(neighbor_group_r !=0xFF                                             )addNeighbor(group, neighbor_data->_path_node_offset+neighbor_group_r , 5);
         if(neighbor_group_rb!=0xFF && (neighbor_group_r!=0xFF || group_b!=0xFF))addNeighbor(group, neighbor_data->_path_node_offset+neighbor_group_rb, 7);
         if(neighbor_group_rf!=0xFF && (neighbor_group_r!=0xFF || group_f!=0xFF))addNeighbor(group, neighbor_data->_path_node_offset+neighbor_group_rf, 7);
      }
   }
   void testBack(Int x)
   {
      Byte group =data->path2D()->_map.pixB(x, 0);
      if(  group!=0xFF)
      {
         Byte group_l          =         data->path2D()->_map.pixB(x-1,                              0),
              group_r          =         data->path2D()->_map.pixB(x+1,                              0),
              neighbor_group_bl=neighbor_data->path2D()->_map.pixB(x-1, World.settings().path2DRes()-1),
              neighbor_group_b =neighbor_data->path2D()->_map.pixB(x  , World.settings().path2DRes()-1),
              neighbor_group_br=neighbor_data->path2D()->_map.pixB(x+1, World.settings().path2DRes()-1);

         if(neighbor_group_b !=0xFF                                             )addNeighbor(group, neighbor_data->_path_node_offset+neighbor_group_b , 5);
         if(neighbor_group_bl!=0xFF && (neighbor_group_b!=0xFF || group_l!=0xFF))addNeighbor(group, neighbor_data->_path_node_offset+neighbor_group_bl, 7);
         if(neighbor_group_br!=0xFF && (neighbor_group_b!=0xFF || group_r!=0xFF))addNeighbor(group, neighbor_data->_path_node_offset+neighbor_group_br, 7);
      }
   }
   void testForward(Int x)
   {
      Byte group =data->path2D()->_map.pixB(x, World.settings().path2DRes()-1);
      if(  group!=0xFF)
      {
         Byte group_l          =         data->path2D()->_map.pixB(x-1, World.settings().path2DRes()-1),
              group_r          =         data->path2D()->_map.pixB(x+1, World.settings().path2DRes()-1),
              neighbor_group_fl=neighbor_data->path2D()->_map.pixB(x-1,                              0),
              neighbor_group_f =neighbor_data->path2D()->_map.pixB(x  ,                              0),
              neighbor_group_fr=neighbor_data->path2D()->_map.pixB(x+1,                              0);

         if(neighbor_group_f !=0xFF                                             )addNeighbor(group, neighbor_data->_path_node_offset+neighbor_group_f , 5);
         if(neighbor_group_fl!=0xFF && (neighbor_group_f!=0xFF || group_l!=0xFF))addNeighbor(group, neighbor_data->_path_node_offset+neighbor_group_fl, 7);
         if(neighbor_group_fr!=0xFF && (neighbor_group_f!=0xFF || group_r!=0xFF))addNeighbor(group, neighbor_data->_path_node_offset+neighbor_group_fr, 7);
      }
   }
   void testCorner(Int x, Int y)
   {
      Byte group =data->path2D()->_map.pixB(x, y);
      if(  group!=0xFF)
      {
         REPA(Offsets)
         {
            Int sx=x+Offsets[i].x, ax=1,
                sy=y+Offsets[i].y, ay=1;
            if( sx<0){ax=0; sx+=World.settings().path2DRes();}else if(sx>=World.settings().path2DRes()){ax=2; sx-=World.settings().path2DRes();}
            if( sy<0){ay=0; sy+=World.settings().path2DRes();}else if(sy>=World.settings().path2DRes()){ay=2; sy-=World.settings().path2DRes();}

            if(ax!=1 || ay!=1) // test only external areas
            if(neighbor_data=data_array[ay][ax])
            {
               Byte neighbor_group =neighbor_data->path2D()->_map.pixB(sx, sy);
               if(  neighbor_group!=0xFF)
               {
                  Byte cost=5;
                  if(Offsets[i].x && Offsets[i].y) // if we're going diagonally, then we need to check horizontal/vertical neighbors
                  {
                     Bool walkable_h=(data_array[1][ax] && data_array[1][ax]->path2D()->_map.pixB(sx,y)!=0xFF),
                          walkable_v=(data_array[ay][1] && data_array[ay][1]->path2D()->_map.pixB(x,sy)!=0xFF);
                     if( !walkable_h && !walkable_v)continue;

                     cost=7;
                  }
                  addNeighbor(group, neighbor_data->_path_node_offset+neighbor_group, cost);
               }
            }
         }
      }
   }

   INLINE Area* getAreaActive(C VecI2 &xy) {return World.areaActive(xy)       ;}
   INLINE Area* getArea      (C VecI2 &xy) {return World._grid.get (xy).data();}

   SetNeighborsHelper(Area &area)
   {
      data=area.data(); REP(data->path2D()->_groups)neighbors[i]=0;

   #if 1 // faster but searches only through active areas
      Area *l =getAreaActive(area.xz()+VecI2(-1, 0)),
           *r =getAreaActive(area.xz()+VecI2( 1, 0)),
           *b =getAreaActive(area.xz()+VecI2( 0,-1)),
           *f =getAreaActive(area.xz()+VecI2( 0, 1)),
           *lb=getAreaActive(area.xz()+VecI2(-1,-1)),
           *lf=getAreaActive(area.xz()+VecI2(-1, 1)),
           *rb=getAreaActive(area.xz()+VecI2( 1,-1)),
           *rf=getAreaActive(area.xz()+VecI2( 1, 1));

      data_array[0][0]=((lb && lb->data() && lb->data()->path2D()) ? lb->data() : null);
      data_array[1][0]=((l  && l ->data() && l ->data()->path2D()) ? l ->data() : null);
      data_array[2][0]=((lf && lf->data() && lf->data()->path2D()) ? lf->data() : null);

      data_array[0][1]=(( b &&  b->data() &&  b->data()->path2D()) ?  b->data() : null);
      data_array[1][1]=                                                data          ;
      data_array[2][1]=(( f &&  f->data() &&  f->data()->path2D()) ?  f->data() : null);

      data_array[0][2]=((rb && rb->data() && rb->data()->path2D()) ? rb->data() : null);
      data_array[1][2]=((r  && r ->data() && r ->data()->path2D()) ? r ->data() : null);
      data_array[2][2]=((rf && rf->data() && rf->data()->path2D()) ? rf->data() : null);
   #else
      Area *l =getArea(area.xz()+VecI2(-1, 0)),
           *r =getArea(area.xz()+VecI2( 1, 0)),
           *b =getArea(area.xz()+VecI2( 0,-1)),
           *f =getArea(area.xz()+VecI2( 0, 1)),
           *lb=getArea(area.xz()+VecI2(-1,-1)),
           *lf=getArea(area.xz()+VecI2(-1, 1)),
           *rb=getArea(area.xz()+VecI2( 1,-1)),
           *rf=getArea(area.xz()+VecI2( 1, 1));

      data_array[0][0]=((lb && lb->_state==AREA_ACTIVE && lb->data() && lb->data()->path2D()) ? lb->data() : null);
      data_array[1][0]=((l  && l ->_state==AREA_ACTIVE && l ->data() && l ->data()->path2D()) ? l ->data() : null);
      data_array[2][0]=((lf && lf->_state==AREA_ACTIVE && lf->data() && lf->data()->path2D()) ? lf->data() : null);

      data_array[0][1]=(( b &&  b->_state==AREA_ACTIVE &&  b->data() &&  b->data()->path2D()) ?  b->data() : null);
      data_array[1][1]=                                                                           data          ;
      data_array[2][1]=(( f &&  f->_state==AREA_ACTIVE &&  f->data() &&  f->data()->path2D()) ?  f->data() : null);

      data_array[0][2]=((rb && rb->_state==AREA_ACTIVE && rb->data() && rb->data()->path2D()) ? rb->data() : null);
      data_array[1][2]=((r  && r ->_state==AREA_ACTIVE && r ->data() && r ->data()->path2D()) ? r ->data() : null);
      data_array[2][2]=((rf && rf->_state==AREA_ACTIVE && rf->data() && rf->data()->path2D()) ? rf->data() : null);
   #endif
   }
};
static void SetNeighbors(Area &area)
{
   if(Area::Data *data=area.data())if(data->path2D())
   {
      AreaPath2D &path=*data->path2D();

      SetNeighborsHelper snh(area);

      // first add external neighbors because they require more testing
      if(snh.neighbor_data=snh.data_array[1][0])for(Int i=1; i<World.settings().path2DRes()-1; i++)snh.testLeft   (i);
      if(snh.neighbor_data=snh.data_array[1][2])for(Int i=1; i<World.settings().path2DRes()-1; i++)snh.testRight  (i);
      if(snh.neighbor_data=snh.data_array[0][1])for(Int i=1; i<World.settings().path2DRes()-1; i++)snh.testBack   (i);
      if(snh.neighbor_data=snh.data_array[2][1])for(Int i=1; i<World.settings().path2DRes()-1; i++)snh.testForward(i);

      // add neighbors in corners
      snh.testCorner(                             0,                              0);
      snh.testCorner(World.settings().path2DRes()-1,                              0);
      snh.testCorner(                             0, World.settings().path2DRes()-1);
      snh.testCorner(World.settings().path2DRes()-1, World.settings().path2DRes()-1);

      // add local neighbors
      REPA(path._neighbor)
      {
         AreaPath2D::Neighbor &neighbor=path._neighbor[i];
         snh.addNeighbor(neighbor.a, data->_path_node_offset+neighbor.b, neighbor.cost);
         snh.addNeighbor(neighbor.b, data->_path_node_offset+neighbor.a, neighbor.cost);
      }

      // add to global list of neighbors
      REPD(g, path._groups)
      {
         PathNode &pn         =World._path_node[data->_path_node_offset+g];
                   pn.nghb_ofs=World._path_neighbor.elms();
         REPD(n,   pn.nghb_num=snh.neighbors[g])World._path_neighbor.New()=snh.neighbor[g][n];
      }
   }
}
static Int PathNodeSiblings(Int i)
{
   for(Int cur=i, num=0; ; num++)
   {
         cur =World._path_node[cur].sibling;
      if(cur==i)return num;
   }
}
void WorldManager::path2DBuild()
{
   // clear path nodes and neighbors
   _path_node    .clear();
   _path_neighbor.clear();

   // set base path nodes
   REPA(_area_active)SetBasePathNodes(*_area_active[i]);

   // set neighbors
   REPA(_area_active)SetNeighbors(*_area_active[i]);

   Memc<UInt            > temp;
   Memc<PathNodeNeighbor> temp_neighbor;

   // join
   for(Int start=0, end; ; start=end)
   {
      // set parents
      end=_path_node.elms(); 
      if(start==end)break;

      for(Int i=start; i<end; i++) // all newly added nodes
      {
         PathNode &pn=_path_node[i];
         if(pn.nghb_num && pn.parent<0) // has neighbors and no parent (no one has taken him yet)
         {
            // check if there's at least one neighbor without parent
            REPD(n, pn.nghb_num)if(_path_node[_path_neighbor[pn.nghb_ofs+n].index].parent<0)goto has_at_least_one_parentless_neighbor;

            temp.New()=i; // add to unassigned
            continue; // all neighbors are taken, so we need to test another node

         has_at_least_one_parentless_neighbor:; // we've found at least one neighbor
            Int        parent_index=_path_node.addNum(1); // create a new parent
            PathNode  &parent      =_path_node[parent_index];
            PathNode  &pn          =_path_node[           i]; // !! after adding element to container old 'pn' reference could point in the wrong place !!
            parent.type    = PN_NODE;
            parent.nghb_num= 0;
            parent.nghb_ofs= 0;
            parent.parent  =-1;
            parent.sibling = parent_index; // self

            parent.node.child=i;

            Int cur=i;
            REPD(n, pn.nghb_num) // all neighbors
            {
               Int       nghb_index=_path_neighbor[pn.nghb_ofs+n].index; // neighbor index
               PathNode &nghb      =_path_node    [   nghb_index]      ; // neighbor node
               if(nghb.parent<0) // if parentless
               {
                  nghb.parent =parent_index; // set parent
                  nghb.sibling=cur;          // set sibling
                  cur         =nghb_index;   // new sibling index
               }
            }

            pn.parent =parent_index; // set parent
            pn.sibling=cur         ; // set current path node sibling to last visited
         }
      }

      // assign those which don't have a parent, to the parent of neighbor with least amount of children
      REPA(temp)
      {
         Int unassigned_index  = temp[i],
             min_siblings_num  = 0,
             min_siblings_index=-1;
         PathNode &pn=_path_node[unassigned_index];
         if(pn.parent<0) // normally temp's should contain nodes with no parent, however there can be a case (in bad algorithm of assigning neighbors) when node A has B written as its neighbor, however B doesn't have A written as its neighbor, then despite the node could have been added to temp, the second node could have assigned him to his parent, that's why make sure that the node doesn't have a parent
         {
            REPD(n, pn.nghb_num) // all neighbors
            {
               Int nghb_index=_path_neighbor[pn.nghb_ofs+n].index;
               if(_path_node[nghb_index].parent>=0) // if the neighbor actually has a parent
               {
                  Int siblings=PathNodeSiblings(nghb_index);
                  if(min_siblings_index<0 || siblings<min_siblings_num){min_siblings_index=nghb_index; min_siblings_num=siblings;}
               }
            }

            if(min_siblings_index>=0) // we've found a neighbor with least amount of siblings
            {
               PathNode &nghb=_path_node[min_siblings_index]; // neighbor node
               pn  .parent =nghb.parent     ; // set the same parent
               pn  .sibling=nghb.sibling    ; // new points on the same which old has pointed to
               nghb.sibling=unassigned_index; // old points on the new one
            }
         }
      }
      temp.clear();

      // set neighbors for newly created parents
      for(Int i=end; i<_path_node.elms(); i++) // for each parent
      {
         PathNode &pn=_path_node[i];
         if(pn.type==PN_NODE && pn.node.child>=0) // if it has children
         {
            // search for parents of children neighbors
            for(Int cur_child=pn.node.child; ; ) // check children
            {
               PathNode &pn_child=_path_node[cur_child];
               REPD(n, pn_child.nghb_num) // for each neighbor of a child
               {
                  PathNodeNeighbor &nghb=_path_neighbor[pn_child.nghb_ofs+n];
                  Int nghb_parent =_path_node[nghb.index].parent; // check neighbors parent
                  if( nghb_parent>=0 && nghb_parent!=i) // if parent exists and it's different than currently processed
                  {
                     REPA(temp_neighbor) // check if parent has already been added as a neighbor
                     {
                        PathNodeNeighbor &tn=temp_neighbor[i];
                        if(tn.index==nghb_parent){MIN(tn.cost, nghb.cost); goto exists;}
                     }
                     temp_neighbor.New().set(nghb_parent, nghb.cost);
                  exists:;
                  }
               }
                  cur_child= pn_child.sibling;    // check next sibling
               if(cur_child==pn.node.child)break; // we've reached the beggining (we've checked all children)
            }

            // add neighbors
            pn.nghb_ofs=_path_neighbor.elms();                        // set neighbor offset
            pn.nghb_num= temp_neighbor.elms();                        // set neighbor count
            REPA(temp_neighbor)_path_neighbor.New()=temp_neighbor[i]; // set neighbors
            temp_neighbor.clear();                                    // clear the temp
         }
      }
   }
}
/******************************************************************************/
AreaPath2D* WorldManager::path2DGet(C VecI2 &xz)
{
   if(Area *area=areaLoaded(xz))if(area->loaded() && area->data())return area->data()->path2D();
   return null;
}
/******************************************************************************/
Bool WorldManager::path2DWalkable(C Vec &pos)
{
   VecI2 xz=worldToArea(pos);
   if(AreaPath2D *path=path2DGet(xz))
   {
      Vec2 pxy=(pos.xz()/areaSize()-Vec2(xz))*settings().path2DRes();
      return path->walkable(Mid(Trunc(pxy.x), 0, path->_map.w()-1),
                            Mid(Trunc(pxy.y), 0, path->_map.h()-1));
   }
   return false;
}
/******************************************************************************/
Int WorldManager::pathGetNode(C Vec &pos, VecI2 &path_xy)
{
   VecI2 xz=worldToArea(pos);
   if(Cell<Area> *g=_grid.find(xz))if(g->data()->loaded())if(Area::Data *data=g->data()->data())if(AreaPath2D *path=data->path2D())
   {
      Vec2 pxy=(pos.xz()/areaSize()-Vec2(xz))*settings().path2DRes();
      path_xy.set(Mid(Trunc(pxy.x), 0, path->_map.w()-1),
                  Mid(Trunc(pxy.y), 0, path->_map.h()-1));

      Byte index=path->_map.pixB(path_xy);
      if(  index<path->_groups) // this already handles !=0xFF (which is being solid)
         return  data->_path_node_offset+index;
   }
   return -1;
}
/******************************************************************************

   Path Graph:
   Starting from top, searching downwards

                          F.top -> T.top                      top
                     F.top-1 -> .. -> T.top-1                  .
                   ..                        ..                .
                F.1 -> .. -> .. -> .. -> .. -> T.1             .
          F.0 -> .. -> .. -> .. -> .. -> .. -> .. -> T.0       0

/******************************************************************************/
struct TempNode
{
   Int  /*path_node, */path_node_last_visited;
   UInt length;

   void set(Int path_node) {/*T.path_node=path_node; */path_node_last_visited=-1; length=0xFFFFFFFF;}
};
static void PrepareTempNodes(WorldManager &world, Memc<TempNode> &temp_node, Int parent)
{
   Int child=world._path_node[parent].node.child;
   for(Int cur=child; ; )
   {
      Int temp_index=temp_node.addNum(1);
      world._path_node[cur].temp=temp_index; // link  PathNode with TempNode
      temp_node[temp_index].set(cur);        // setup TempNode
      cur=world._path_node[cur].sibling;     // take  sibling
      if(cur==child)break;                   // we've reached the start
   }
}
Bool WorldManager::pathFindFast(Int node_from, Int node_to, Memc<UInt> &path)
{
   path.clear();

   if(InRange(node_from, _path_node)
   && InRange(node_to  , _path_node))
   {
      if(node_from==node_to)return true; // the same nodes

      Memc<UInt> from_start, to_end; // stack of first and last node levels
      for(Int from_cur=node_from, to_cur=node_to; ; )
      {
         Int from_parent=_path_node[from_cur].parent,
               to_parent=_path_node[to_cur  ].parent;
         if(from_parent<0 || to_parent<0)break; // no parents

         // add to level stack
         from_start.add(from_cur);
           to_end  .add(  to_cur);

         if(from_parent==to_parent) // found the same parent
         {
            Memc<TempNode> temp_node;
            Memc<UInt    > nodes[2] ;
            Memc<UInt    > active   ;
            Bool           cur=   0,
                           top=!cur;

            for(;;)
            {
               // remove from the stack the highest elements
               from_cur=from_start.pop();
                 to_cur=  to_end  .pop();

               // go from 'from_cur' to 'nodes[top]' to 'to_cur' writing in 'nodes[cur]' all neighbors passed by (in order) (skipping 'from_cur' and 'to_cur')
               {
                  Bool found;
                  UInt found_length;
                  Int  found_index,
                       found_last_visited,
                       cur_node=from_cur;

                  // go through 'nodes[top]' (these are following parents of 'cur_node' neighbors leading to 'to_cur', following parents are connected with each other)
                  FREPD(p, nodes[top].elms()+1) // +1 means guaranteed walk to 'to_cur' parent
                  {
                     Int cur_parent=_path_node[cur_node].parent,
                         new_parent=((p<nodes[top].elms()) ? nodes[top][p] : _path_node[to_cur].parent);

                     // go from 'cur_node' to nearest child of 'new_parent' parent, we can walk only on 'cur_parent'
                     if(cur_parent!=new_parent) // make sure that we're not already in the target parent
                     {
                        // prepare 'temp_node' on which we'll be walking (only from 'cur_parent')
                        PrepareTempNodes(T, temp_node, cur_parent);

                        // set start
                        temp_node[_path_node[cur_node].temp].length=0;
                        active.add(cur_node);

                        // start searching
                        for(found=false; ; )
                        {
                           REPAD(a, active) // order is important
                           {
                              cur_node=active[a];
                              PathNode &pn        =_path_node[cur_node];
                              UInt      cur_length= temp_node[pn .temp].length+1;

                              REPD(n, pn.nghb_num) // check all neighbors
                              {
                                 PathNodeNeighbor &nghb_info =_path_neighbor[pn.nghb_ofs+n];
                                 Int               nghb_index=nghb_info.index;
                                 PathNode         &nghb      =_path_node[nghb_index];
                                 UInt              new_length=cur_length+nghb_info.cost;

                                 if(nghb.parent==cur_parent) // it can be walked
                                 {
                                    if(new_length<temp_node[nghb.temp].length) // we've reached faster than using other way
                                    {
                                       temp_node[nghb.temp].length                =new_length;
                                       temp_node[nghb.temp].path_node_last_visited=cur_node;
                                       active.include(nghb_index);
                                    }
                                 }else
                                 if(nghb.parent==new_parent) // target parent
                                 {
                                    if(nghb_index==to_cur)new_length--; // if we've encountered the target then bonus the length
                                    if(!found || new_length<found_length)
                                    {
                                       found       =true;
                                       found_length=new_length;
                                       found_index =nghb_index;
                                       found_last_visited=cur_node;
                                    }
                                 }
                              }
                              active.remove(a);
                           }
                           if(found)
                           {
                              // iterate all visited nodes saving them (from end to start), use container 'active'
                              // don't write where we've started, but write where we've ended up (so target without the first one)
                              active.clear().add(found_index);
                              for(;;)
                              {
                                 Int last_visited=temp_node[_path_node[found_last_visited].temp].path_node_last_visited;
                                 if( last_visited<0)break; // 'found_last_visited' was first because no one visited him, and since we don't add the first so let's break
                                 active.add(found_last_visited);
                                 found_last_visited=last_visited;
                              }

                              // add 'active' in the right order to 'nodes[cur]'
                              REPA(active)nodes[cur].add(active[i]);

                              cur_node=found_index; // set current as last found
                              break;
                           }
                           if(!active.elms())return false; // shouldn't happen
                        }
                        active   .clear();
                        temp_node.clear();
                     }
                  }

                  // go to 'to_cur'
                  if(cur_node==to_cur) // we're already there
                  {
                     if(nodes[cur].elms() && nodes[cur].last()==to_cur)nodes[cur].removeLast(); // if one node was added too much
                  }else
                  {
                     // we're already at 'to_parent' parent, we're searching path from 'cur_node' to 'to_cur' without writing the first and last one on the road (without 'cur_node' and 'to_cur')

                     // prepare 'temp_node' on which we'll be walking (only from 'to_parent')
                     PrepareTempNodes(T, temp_node, to_parent);

                     // set start
                     temp_node[_path_node[cur_node].temp].length=0;
                     active.add(cur_node);

                     // start searching
                     for(found=false; ; )
                     {
                        REPAD(a, active) // order is important
                        {
                           cur_node=active[a];
                           PathNode &pn        =_path_node[cur_node];
                           UInt      cur_length= temp_node[pn .temp].length+1;

                           REPD(n, pn.nghb_num) // check all neighbors
                           {
                              PathNodeNeighbor &nghb_info =_path_neighbor[pn.nghb_ofs+n];
                              Int               nghb_index=nghb_info.index;
                              PathNode         &nghb      =_path_node[nghb_index];
                              UInt              new_length=cur_length+nghb_info.cost;

                              if(nghb.parent==to_parent                  // consider only nodes in the final parent 'to_parent'
                              && new_length<temp_node[nghb.temp].length) // we've reached faster than using a different way
                              {
                                 temp_node[nghb.temp].length                =new_length;
                                 temp_node[nghb.temp].path_node_last_visited=cur_node;

                                 if(nghb_index==to_cur) // found, this is the last element
                                 {
                                    found=true;
                                    found_last_visited=cur_node;
                                 }else
                                 {
                                    active.include(nghb_index);
                                 }
                              }
                           }
                           active.remove(a);
                        }
                        if(found)
                        {
                           // iterate all visited nodes saving them (from end to start), use container 'active'
                           // don't write where we've started and when we're finishing (skip last and first ones)
                           active.clear();
                           for(;;)
                           {
                              Int last_visited=temp_node[_path_node[found_last_visited].temp].path_node_last_visited;
                              if( last_visited<0)break; // 'found_last_visited' was first because no one visited him, and since we don't add the first so let's break
                              active.add(found_last_visited);
                              found_last_visited=last_visited;
                           }

                           // add 'active' in the right order to 'nodes[cur]'
                           REPA(active)nodes[cur].add(active[i]);

                           break; // found last element on this level
                        }
                        if(!active.elms())return false; // shouldn't happen
                     }
                     active   .clear();
                     temp_node.clear();
                  }
               }

               // finish
               if(!from_start.elms() || !to_end.elms())
               {
                  // nodes[cur] is a path
                                  path.add(node_to      ); // add target node
                  REPA(nodes[cur])path.add(nodes[cur][i]); // add in reversed order
                  return true;
               }
               Swap(top, cur);
               nodes[cur].clear();
               from_parent=from_cur;
                 to_parent=  to_cur;
            }

         #if DEBUG
          //pathDraw(from_cur, GREEN);
          //pathDraw(  to_cur, RED  );
         #endif
            break;
         }

         from_cur=from_parent;
           to_cur=  to_parent;
      }
   }
   return false;
}
/******************************************************************************/
Bool WorldManager::pathFind(Int node_from, Int node_to, Memc<UInt> &path)
{
   path.clear();

   if(InRange(node_from, _path_node)
   && InRange(node_to  , _path_node))
   {
      if(node_from==node_to)return true; // the same nodes

      // check if it's possible to find a path (they have mutual ancestors)
      for(Int from_parent=node_from, to_parent=node_to; ; )
      {
            from_parent=_path_node[from_parent].parent;
              to_parent=_path_node[  to_parent].parent;
         if(from_parent<0 ||       to_parent<0)return false; // no parents
         if(from_parent   ==       to_parent  )break       ; // found the same parent
      }

      // update iterations (reset nodes if needed)
      if(++_path_iteration==0) // we've reset the counter (got back to zero again)
      {
         FREPA(_path_node) // reset nodes, from the start to skip parents
         {
            PathNode &pn=_path_node[i]; if(pn.type!=PN_AREA)break; // we've encountered a parent so break
            pn.iteration=0; // reset iteration
         }
        _path_iteration=1;
      }

      Memc<PathNode*> active;

      // setup first step
     _path_node[node_from].iteration=_path_iteration;
     _path_node[node_from].length   =0;
     _path_node[node_from].src      =null;
      active.clear().add(&_path_node[node_from]);

      // start searching
      UInt      found_length;
      PathNode *found=null;
      for(Int step=0; ; step++)
      {
         REPA(active) // order is important
         {
            PathNode *p=active[i];          // for each walker
            UInt      cur_length=p->length; // total length travelled by the walker
            REPD(n, p->nghb_num)            // try going in each direction
            {
               PathNodeNeighbor &pnn=_path_neighbor[p->nghb_ofs+n];
               PathNode         &to =_path_node    [pnn.index]; // target node
               UInt              new_length=cur_length+pnn.cost;

               // zero if not yet used
               if(to.iteration!=_path_iteration)
               {
                  to.iteration    =_path_iteration;
                  to.added_in_step=-1;
                  to.length       =UINT_MAX;
               }

               if(new_length<to.length)
               {
                  if(pnn.index==node_to)
                  {
                     to.length=new_length;
                     to.src   =p;
                     if(!found || new_length<found_length)
                     {
                        found       =&to;
                        found_length=new_length;
                     }
                  }else
                  {
                     to.length=new_length;
                     to.src   =p;
                     if(to.added_in_step!=step) // to not add multiple times in the same step
                     {
                        to.added_in_step=step;
                        active.add(&to);
                     }
                  }
               }
            }
            active.remove(i);
         }
         if(found) // found path
         {
            // build path
            for(;;)
            {
               PathNode *src=found->src;
               if(      !src)break;
               path.add(found->node_index);
               found=src;
            }
            return true;
         }
         if(!active.elms())return false;
      }
   }
   return false;
}
/******************************************************************************/
void WorldManager::pathDrawArea(Area &area, Byte index, C Color &color)
{
   if(area.loaded() && area.data())if(AreaPath2D *path=area.data()->path2D())
   {
      Image &height=area.data()->height;
      Flt    scale =Flt(height.w()-1)/settings().path2DRes();

      VI.color(color);
      SetOneMatrix(Matrix(Vec(areaSize()/settings().path2DRes(), 1, areaSize()/settings().path2DRes()), (area.xz()*areaSize()).x0y()));
      REPD(y, path->_map.h())
      REPD(x, path->_map.w())if(path->_map.pixB(x, y)==index)
      {
         Quad q;
         q.p[0].set(x  , height.pixelF( x   *scale, (y+1)*scale), y+1);
         q.p[1].set(x+1, height.pixelF((x+1)*scale, (y+1)*scale), y+1);
         q.p[2].set(x+1, height.pixelF((x+1)*scale,  y   *scale), y  );
         q.p[3].set(x  , height.pixelF( x   *scale,  y   *scale), y  );
         VI.quad(q);
      }
      VI.end();
   }
}
void WorldManager::pathDraw(Int node, C Color &color)
{
   if(InRange(node, _path_node))
   {
      PathNode &pn=_path_node[node];
      switch(pn.type)
      {
         case PN_NODE:
         {
            Int child=pn.node.child;
            for(Int c=child; c>=0; )
            {
               pathDraw(c, color);
                  c= _path_node[c].sibling; // grab the next one
               if(c==child)break;           // we've reached the start
            }
         }break;

         case PN_AREA:
         {
            if(Cell<Area> *cell=_grid.find(pn.area.xy))pathDrawArea(*cell->data(), pn.area.index, color);
         }break;
      }
   }
}
void WorldManager::pathDrawNghb(Int node, C Color &color)
{
   if(InRange(node, _path_node))
   {
      PathNode &pn=_path_node[node];
      REP(pn.nghb_num)pathDraw(_path_neighbor[pn.nghb_ofs+i].index, color);
   }
}
void WorldManager::pathDrawBlock(C Color &color)
{
   REPA(_area_active)pathDrawArea(*_area_active[i], 0xFF, color);
}
/******************************************************************************/
}}
/******************************************************************************/
