/******************************************************************************/
#include "stdafx.h"
namespace EE{
#define MAX_NORMAL 1743392200 // -MAX_NORMAL .. MAX_NORMAL is the biggest normal cell that doesn't overflow
/******************************************************************************

   Grid supports all indexes (INT_MIN .. INT_MAX)

   Sample Grid:

   ((-13 -12 -11) (-10 -9 -8) (-7 -6 -5))  ((-4 -3 -2) (-1 0 1) (2 3 4))  ((5 6 7) (8 9 10) (11 12 13))
           \            |         /              \         |       /          \       |         /
            \           |        /                \        |      /            \      |        /
             \          |       /                  \       |     /              \     |       /
                -13, -11, -8, -5                     -4, -2, 1, 4                 5, 7, 10, 13
                                         \                 |            /
                                          \                |           /
                                           \               |          /
                                                     -13 -5 4 13




                                         -40 -14     -13 -5 4 13       14 40

/******************************************************************************

   Normally _x[1], _x[2], _x[3] don't overlap, however around INT_MIN/INT_MAX there's special case when they can, because of int range clamping inside 'createAsParent'
            _y[1], _y[2], _y[3]
      Because of this, all codes should check first for _x[1], then for _x[2], then for _x[3], and not for example:
         -some       codes check first for _x[1], then for _x[2], then for _x[3]
         -some other codes check first for _x[3], then for _x[2], then for _x[1]

/******************************************************************************/
// MANAGE
/******************************************************************************/
void _Cell::del(_Grid &grid)
{
   // delete children
   REPA(_g)if(_g[i])_g[i]->del(grid);

   // first delete the data because as they exist in this area, they still can reference the area
   if(_data)
   {
      if(grid._del)grid._del(_data);else Exit("'_Cell.del' Cell has data but no data destructor");
   }

   // then remove from grid and parent
   if(final() && Cuts(xy(), grid._fast_access_rect))grid.fastAccessCell(xy())=null;
   if(_parent)
   {
     _Cell *(&pg)[9]=_parent->_g;
      REPA(pg)if(pg[i]==this){Delete(pg[i]); return;} // !! 'this' is now DELETED, therefore return IMMEDIATELY !!
   }
}
_Cell* _Cell::create(Int x, Int y, _Cell *parent, _Grid &grid)
{
   T._x[0]=T._x[1]=T._x[2]=T._x[3]=x;
   T._y[0]=T._y[1]=T._y[2]=T._y[3]=y;

   T._parent=parent;

   if(Cuts(xy(), grid._fast_access_rect))grid.fastAccessCell(xy())=this;

   if(grid._new)grid._new(_data, VecI2(x, y), grid.user); // call this at the end, when the _Cell is ready

   return this;
}
_Cell* _Cell::create(Int x0, Int y0, Int x1, Int y1, _Cell *parent, _Grid &grid)
{
   // need to check for x0>=x1 and y0>=y1 because outside MAX_NORMAL range, one index can overlap, but other not (for example x0=x1=-9, but y0=0, y1=4, x is the same, but y different)
   if(x0>=x1){if(y0>=y1)return create(x1, y1, parent, grid); _x[0]=_x[1]=_x[2]=_x[3]=x1;}else{_x[0]=x0; _x[3]=x1; UInt dx=x1-x0; if(dx>=3)dx-=2; dx/=3; _x[1]=x0+dx; _x[2]=x0+dx*2+1;} // {use x1 instead of x0 beacuse x0 is set as parent+1 and may be out of range} else {this avoids under/over-flow: _x[1]=x0+(x1-x0-2)/3; _x[2]=x0+(x1-x0-2)/3*2+1;}
   if(y0>=y1){                                               _y[0]=_y[1]=_y[2]=_y[3]=y1;}else{_y[0]=y0; _y[3]=y1; UInt dy=y1-y0; if(dy>=3)dy-=2; dy/=3; _y[1]=y0+dy; _y[2]=y0+dy*2+1;} // {use y1 instead of y0 beacuse y0 is set as parent+1 and may be out of range} else {this avoids under/over-flow: _y[1]=y0+(y1-y0-2)/3; _y[2]=y0+(y1-y0-2)/3*2+1;}
   T._parent=parent;
#if 0 // normally this should be:
   DEBUG_ASSERT(_x[0]<=_x[1] && _x[1]<_x[2] && _x[2]<_x[3]
             && _y[0]<=_y[1] && _y[1]<_y[2] && _y[2]<_y[3], "Grid invalid indexes");
#else // however around INT_MIN/INT_MAX there's special case where indexes overlap
   DEBUG_ASSERT(_x[0]<=_x[1] && _x[1]<=_x[2] && _x[2]<=_x[3]
             && _y[0]<=_y[1] && _y[1]<=_y[2] && _y[2]<=_y[3], "Grid invalid indexes");
#endif
   return this;
}
_Cell* _Cell::createAsParent(_Cell *child, _Grid &grid)
{
   // align
   Int  cx=child->_x[0],
        cy=child->_y[0];
   Long x=cx, y=cy, div=ULong(Long(child->_x[3])-x+1)*3, div_2=div/2;
   // formula below works as 'AlignRound', it's needed so the parent will contain the child, it will always create parents at the same coordinates regardless where the first cell was located (for example, if first element is -13, then parent will always be correctly aligned, and located towards zero)
   if(x>0)x+=div_2;else x-=div_2; x=(x/div)*div;
   if(y>0)y+=div_2;else y-=div_2; y=(y/div)*div;

   // create new root
   Long x0=x-div_2, x1=x+div_2,
        y0=y-div_2, y1=y+div_2;
   if(x0<INT_MIN || x1>INT_MAX
   || y0<INT_MIN || y1>INT_MAX) // if any index overflows
   {
      DEBUG_ASSERT(cx==-MAX_NORMAL && child->_x[3]==MAX_NORMAL
                && cy==-MAX_NORMAL && child->_y[3]==MAX_NORMAL, "invalid max child size");
      // setup middle to be exactly as the child
      T._x[0]=INT_MIN; T._x[1]=cx-1; T._x[2]=child->_x[3]; T._x[3]=INT_MAX;
      T._y[0]=INT_MIN; T._y[1]=cy-1; T._y[2]=child->_y[3]; T._y[3]=INT_MAX;
      T._parent=null;
   }else create(x0, y0, x1, y1, null, grid);

   // link
   DEBUG_ASSERT(child->_x[0]>=T._x[0] && child->_x[3]<=T._x[3]
             && child->_y[0]>=T._y[0] && child->_y[3]<=T._y[3], "Grid parent doesn't contain child");
   if(cx<=T._x[1])
   {
      if(cy<=T._y[1])_lb=child;else
      if(cy<=T._y[2])_l =child;else
                     _lf=child;
   }else
   if(cx<=T._x[2])
   {
      if(cy<=T._y[1]) _b=child;else
      if(cy<=T._y[2]) _c=child;else
                      _f=child;
   }else
   {
      if(cy<=T._y[1])_rb=child;else
      if(cy<=T._y[2])_r =child;else
                     _rf=child;
   }
   child->_parent=this;
   return this;
}
/******************************************************************************/
// GET / FIND
/******************************************************************************/
_Cell* _Cell::get(C VecI2 &xy, _Grid &grid)
{
   if(xy.x>=T._x[0] && xy.x<=T._x[3]
   && xy.y>=T._y[0] && xy.y<=T._y[3])
   {
      if(final())return this;
      if(xy.x<=T._x[1])
      {
         if(xy.y<=T._y[1]){if(!_lb)New(_lb)->create(T._x[0]  , T._y[0]  , T._x[1], T._y[1], this, grid); return _lb->get(xy, grid);} // first allocate and assign, then create (in case child accesses grid)
         if(xy.y<=T._y[2]){if(!_l )New(_l )->create(T._x[0]  , T._y[1]+1, T._x[1], T._y[2], this, grid); return _l ->get(xy, grid);} // first allocate and assign, then create (in case child accesses grid)
                          {if(!_lf)New(_lf)->create(T._x[0]  , T._y[2]+1, T._x[1], T._y[3], this, grid); return _lf->get(xy, grid);} // first allocate and assign, then create (in case child accesses grid)
      }
      if(xy.x<=T._x[2])
      {
         if(xy.y<=T._y[1]){if(! _b)New( _b)->create(T._x[1]+1, T._y[0]  , T._x[2], T._y[1], this, grid); return  _b->get(xy, grid);} // first allocate and assign, then create (in case child accesses grid)
         if(xy.y<=T._y[2]){if(! _c)New( _c)->create(T._x[1]+1, T._y[1]+1, T._x[2], T._y[2], this, grid); return  _c->get(xy, grid);} // first allocate and assign, then create (in case child accesses grid)
                          {if(! _f)New( _f)->create(T._x[1]+1, T._y[2]+1, T._x[2], T._y[3], this, grid); return  _f->get(xy, grid);} // first allocate and assign, then create (in case child accesses grid)
      }
      {
         if(xy.y<=T._y[1]){if(!_rb)New(_rb)->create(T._x[2]+1, T._y[0]  , T._x[3], T._y[1], this, grid); return _rb->get(xy, grid);} // first allocate and assign, then create (in case child accesses grid)
         if(xy.y<=T._y[2]){if(!_r )New(_r )->create(T._x[2]+1, T._y[1]+1, T._x[3], T._y[2], this, grid); return _r ->get(xy, grid);} // first allocate and assign, then create (in case child accesses grid)
                          {if(!_rf)New(_rf)->create(T._x[2]+1, T._y[2]+1, T._x[3], T._y[3], this, grid); return _rf->get(xy, grid);} // first allocate and assign, then create (in case child accesses grid)
      }
   }
   return null;
}
_Cell* _Cell::find(C VecI2 &xy)
{
   if(xy.x>=T._x[0] && xy.x<=T._x[3]
   && xy.y>=T._y[0] && xy.y<=T._y[3])
   {
      if(final())return this;
      if(xy.x<=T._x[1])
      {
         if(xy.y<=T._y[1])return _lb ? _lb->find(xy) : null;
         if(xy.y<=T._y[2])return _l  ? _l ->find(xy) : null;
                          return _lf ? _lf->find(xy) : null;
      }
      if(xy.x<=T._x[2])
      {
         if(xy.y<=T._y[1])return  _b ?  _b->find(xy) : null;
         if(xy.y<=T._y[2])return  _c ?  _c->find(xy) : null;
                          return  _f ?  _f->find(xy) : null;
      }
      {
         if(xy.y<=T._y[1])return _rb ? _rb->find(xy) : null;
         if(xy.y<=T._y[2])return _r  ? _r ->find(xy) : null;
                          return _rf ? _rf->find(xy) : null;
      }
   }
   return null;
}
/******************************************************************************/
// SIZE
/******************************************************************************/
Bool _Cell::getLeft(Int &value)C
{
   if(final())
   {
      if(x()<value)
      {
         value=x();
         return true;
      }
   }else
   {
      Bool ok=false;
      if(_x[0]<value) // some elements in range x0..x1 can decrease 'value'
      {
         if(_lf)ok|=_lf->getLeft(value);
         if(_l )ok|=_l ->getLeft(value);
         if(_lb)ok|=_lb->getLeft(value);
         if( ok)return true;
      }
      if(_x[1]+1<value) // some elements in range x1+1..x2 can decrease 'value'
      {
         if(_f)ok|=_f->getLeft(value);
         if(_c)ok|=_c->getLeft(value);
         if(_b)ok|=_b->getLeft(value);
         if(ok)return true;
      }
      if(_x[2]+1<value) // some elements in range x2+1..x3 can decrease 'value'
      {
         if(_rf)ok|=_rf->getLeft(value);
         if(_r )ok|=_r ->getLeft(value);
         if(_rb)ok|=_rb->getLeft(value);
         if( ok)return true;
      }
   }
   return false;
}
Bool _Cell::getRight(Int &value)C
{
   if(final())
   {
      if(x()>value)
      {
         value=x();
         return true;
      }
   }else
   {
      Bool ok=false;
      if(_x[3]>value) // some elements in range x2+1..x3 can increase 'value'
      {
         if(_rf)ok|=_rf->getRight(value);
         if(_r )ok|=_r ->getRight(value);
         if(_rb)ok|=_rb->getRight(value);
         if( ok)return true;
      }
      if(_x[2]>value) // some elements in range x1+1..x2 can increase 'value'
      {
         if(_f)ok|=_f->getRight(value);
         if(_c)ok|=_c->getRight(value);
         if(_b)ok|=_b->getRight(value);
         if(ok)return true;
      }
      if(_x[1]>value) // some elements in range x0..x1 can increase 'value'
      {
         if(_lf)ok|=_lf->getRight(value);
         if(_l )ok|=_l ->getRight(value);
         if(_lb)ok|=_lb->getRight(value);
         if( ok)return true;
      }
   }
   return false;
}
Bool _Cell::getBottom(Int &value)C
{
   if(final())
   {
      if(y()<value)
      {
         value=y();
         return true;
      }
   }else
   {
      Bool ok=false;
      if(_y[0]<value) // some elements in range y0..y1 can decrease 'value'
      {
         if(_lb)ok|=_lb->getBottom(value);
         if( _b)ok|= _b->getBottom(value);
         if(_rb)ok|=_rb->getBottom(value);
         if( ok)return true;
      }
      if(_y[1]+1<value) // some elements in range y1+1..y2 can decrease 'value'
      {
         if(_l)ok|=_l->getBottom(value);
         if(_c)ok|=_c->getBottom(value);
         if(_r)ok|=_r->getBottom(value);
         if(ok)return true;
      }
      if(_y[2]+1<value) // some elements in range y2+1..y3 can decrease 'value'
      {
         if(_lf)ok|=_lf->getBottom(value);
         if( _f)ok|= _f->getBottom(value);
         if(_rf)ok|=_rf->getBottom(value);
         if( ok)return true;
      }
   }
   return false;
}
Bool _Cell::getTop(Int &value)C
{
   if(final())
   {
      if(y()>value)
      {
         value=y();
         return true;
      }
   }else
   {
      Bool ok=false;
      if(_y[3]>value) // some elements in range y2+1..y3 can increase 'value'
      {
         if(_lf)ok|=_lf->getTop(value);
         if( _f)ok|= _f->getTop(value);
         if(_rf)ok|=_rf->getTop(value);
         if( ok)return true;
      }
      if(_y[2]>value) // some elements in range y1+1..y2 can increase 'value'
      {
         if(_l)ok|=_l->getTop(value);
         if(_c)ok|=_c->getTop(value);
         if(_r)ok|=_r->getTop(value);
         if(ok)return true;
      }
      if(_y[1]>value) // some elements in range y0..y1 can increase 'value'
      {
         if(_lb)ok|=_lb->getTop(value);
         if( _b)ok|= _b->getTop(value);
         if(_rb)ok|=_rb->getTop(value);
         if( ok)return true;
      }
   }
   return false;
}
/******************************************************************************/
// FUNC
/******************************************************************************/
void _Cell::func(void func(_Cell &cell, Ptr user), Ptr user)
{
   if(final())func(T, user);else
   FREPA(_g)if(_g[i])_g[i]->func(func, user); // FREPA is faster in this case
}
void _Cell::func(C RectI &rect, void func(_Cell &cell, Ptr user), Ptr user)
{
   if(rect.min.x<=T._x[3] && rect.max.x>=T._x[0]
   && rect.min.y<=T._y[3] && rect.max.y>=T._y[0])
   {
      if(final())func(T, user);else
      FREPA(_g)if(_g[i])_g[i]->func(rect, func, user); // FREPA is faster in this case
   }
}
void _Cell::funcCreate(C RectI &rect, void func(_Cell &cell, Ptr user), Ptr user, _Grid &grid)
{
   if(rect.min.x<=T._x[3] && rect.max.x>=T._x[0]
   && rect.min.y<=T._y[3] && rect.max.y>=T._y[0])
   {
      if(final())func(T, user);else
      {
         if(rect.min.x<=T._x[1]/* && rect.max.x>=T._x[0]*/)
         {
            if(  rect.min.y<=T._y[1]/* &&   rect.max.y>=T._y[0]*/){if(!_lb)New(_lb)->create(T._x[0]  , T._y[0]  , T._x[1], T._y[1], this, grid); _lb->funcCreate(rect, func, user, grid);} // first allocate and assign, then create (in case child accesses grid)
            if(  rect.min.y<=T._y[2]   &&   rect.max.y> T._y[1]  ){if(!_l )New(_l )->create(T._x[0]  , T._y[1]+1, T._x[1], T._y[2], this, grid); _l ->funcCreate(rect, func, user, grid);} // first allocate and assign, then create (in case child accesses grid)
            if(/*rect.min.y<=T._y[3]   && */rect.max.y> T._y[2]  ){if(!_lf)New(_lf)->create(T._x[0]  , T._y[2]+1, T._x[1], T._y[3], this, grid); _lf->funcCreate(rect, func, user, grid);} // first allocate and assign, then create (in case child accesses grid)
         }
         if(rect.min.x<=T._x[2] && rect.max.x>T._x[1])
         {
            if(  rect.min.y<=T._y[1]/* &&   rect.max.y>=T._y[0]*/){if(! _b)New( _b)->create(T._x[1]+1, T._y[0]  , T._x[2], T._y[1], this, grid);  _b->funcCreate(rect, func, user, grid);} // first allocate and assign, then create (in case child accesses grid)
            if(  rect.min.y<=T._y[2]   &&   rect.max.y> T._y[1]  ){if(! _c)New( _c)->create(T._x[1]+1, T._y[1]+1, T._x[2], T._y[2], this, grid);  _c->funcCreate(rect, func, user, grid);} // first allocate and assign, then create (in case child accesses grid)
            if(/*rect.min.y<=T._y[3]   && */rect.max.y> T._y[2]  ){if(! _f)New( _f)->create(T._x[1]+1, T._y[2]+1, T._x[2], T._y[3], this, grid);  _f->funcCreate(rect, func, user, grid);} // first allocate and assign, then create (in case child accesses grid)
         }
         if(/*rect.min.x<=T._x[3] && */rect.max.x>T._x[2])
         {
            if(  rect.min.y<=T._y[1]/* &&   rect.max.y>=T._y[0]*/){if(!_rb)New(_rb)->create(T._x[2]+1, T._y[0]  , T._x[3], T._y[1], this, grid); _rb->funcCreate(rect, func, user, grid);} // first allocate and assign, then create (in case child accesses grid)
            if(  rect.min.y<=T._y[2]   &&   rect.max.y> T._y[1]  ){if(!_r )New(_r )->create(T._x[2]+1, T._y[1]+1, T._x[3], T._y[2], this, grid); _r ->funcCreate(rect, func, user, grid);} // first allocate and assign, then create (in case child accesses grid)
            if(/*rect.min.y<=T._y[3]   && */rect.max.y> T._y[2]  ){if(!_rf)New(_rf)->create(T._x[2]+1, T._y[2]+1, T._x[3], T._y[3], this, grid); _rf->funcCreate(rect, func, user, grid);} // first allocate and assign, then create (in case child accesses grid)
         }
      }
   }
}
/******************************************************************************/
// GRID
/******************************************************************************/
static void SetFastAccess(_Cell &cell, Ptr user)
{
  _Grid &grid=*(_Grid*)user;
   grid.fastAccessCell(cell.xy())=&cell;
}
/******************************************************************************/
static _Cell& GetCell(_Cell* &root, C VecI2 &xy, _Grid &grid)
{
   if(root)
   {
   parent:
      // set new roots on top of current one until we can hold the new element below root
      for(;;)
      {
         if(_Cell *get=root->get(xy, grid))return *get; // if root contains the element
         DEBUG_ASSERT(root->x()!=INT_MIN, "failed to find cell"); // root.x==INT_MIN means the biggest root possible
        _Cell *child=root;
         New(root)->createAsParent(child, grid); // creating parent doesn't create data objects, so whether we first create or assign (the order) it doesn't matter
      }
   }else
   {
      if(xy.x<-MAX_NORMAL || xy.x>MAX_NORMAL  // check each component separately (instead of using 'Abs' because of "Abs(INT_MIN)!=-INT_MIN")
      || xy.y<-MAX_NORMAL || xy.y>MAX_NORMAL) // if the cell to be created first lies outside of normal range, then we first need to create at least one cell in the normal area, because 'createAsParent' will work incorrectly for cells outside of normal range ('div' calculation)
      {
         New(root)->create(-MAX_NORMAL, -MAX_NORMAL, MAX_NORMAL, MAX_NORMAL, null, grid);
         goto parent;
      }
      New(root)->create(xy.x, xy.y, null, grid); // first allocate and assign, then create (in case child accesses grid)
      return *root;
   }
}
/******************************************************************************/
void   _Grid:: del(             )  {if(_root)_root->del(T);                Delete(_root);}
void   _Grid:: del( _Cell  *cell)  {if( cell) cell->del(T); if(cell==_root)Delete(_root);}
_Cell& _Grid:: get(C VecI2 &xy  )  {if(Cuts(xy, _fast_access_rect))if(_Cell *cell=fastAccessCell(xy))return *cell; return GetCell(_root,      xy, T)    ;} // when using fast_access lookup then return only if it was found, otherwise create it
_Cell* _Grid::find(C VecI2 &xy  )C {if(Cuts(xy, _fast_access_rect))return         fastAccessCell(xy)             ; return _root ? _root->find(xy) : null;} // when using fast_access lookup then always return because we're not creating it when not found
Bool   _Grid::size(  RectI &rect)C
{
   // set to max invalid first, because methods below will adjust existing values
   rect.min=INT_MAX;
   rect.max=INT_MIN;
   if(_root)
   {
     _root->getLeft (rect.min.x); _root->getBottom(rect.min.y);
     _root->getRight(rect.max.x); _root->getTop   (rect.max.y);

      if(rect.valid())return true;
   }
   rect.zero(); return false;
}
void _Grid::fastAccess(C RectI *rect)
{
   RectI invalid; if(!rect || !rect->valid())rect=&invalid.set(0, 0, -1, -1);
   if(_fast_access_rect!=*rect)
   {
     _fast_access_mul = rect->w()+1;
     _fast_access_rect=*rect;
     _fast_access_cell.setNum(_fast_access_mul*(rect->h()+1));
      ZeroN(_fast_access_cell.data(), _fast_access_cell.elms()); // set to null initially in case not all cells are created in that area
      func(*rect, SetFastAccess, this);
   }
}

void _Grid::func      (               void func(_Cell &cell, Ptr user), Ptr user) {if(_root)_root->func(      func, user);}
void _Grid::func      (C RectI &rect, void func(_Cell &cell, Ptr user), Ptr user) {if(_root)_root->func(rect, func, user);}
void _Grid::funcCreate(C RectI &rect, void func(_Cell &cell, Ptr user), Ptr user)
{
   // create or move root above to cover all required areas
   if(!_root || _root->_x[0]>rect.min.x || _root->_y[0]>rect.min.y)GetCell(_root, rect.min, T); // get min corner
   if(!_root || _root->_x[3]<rect.max.x || _root->_y[3]<rect.max.y)GetCell(_root, rect.max, T); // get max corner

   // now create and call the callback
   if(_root)_root->funcCreate(rect, func, user, T);
}

static void ListCells(_Cell &cell, Memt<_Cell*> &cells)
{
   cells.add(&cell);
}
void _Grid::mtFunc(Threads &threads, void func(_Cell &cell, Ptr user, Int thread_index), Ptr user)
{
   if(_root)
   {
      Memt<_Cell*> cells; _root->func(ListCells, cells); // get all valid cells
      threads.process1(cells, func, user);
   }
}
void _Grid::mtFunc(Threads &threads, C RectI &rect, void func(_Cell &cell, Ptr user, Int thread_index), Ptr user)
{
   if(_root)
   {
      Memt<_Cell*> cells; _root->func(rect, ListCells, cells); // get all valid cells
      threads.process1(cells, func, user);
   }
}
/******************************************************************************/
}
/******************************************************************************/
