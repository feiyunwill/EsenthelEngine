/******************************************************************************/
#include "stdafx.h"
namespace EE{
/******************************************************************************/
void PathFind::Pixel::create(Int x, Int y) {xy.set(x, y); flag=0; iteration=0;}
/******************************************************************************/
     PathFind::PathFind() {zero();}
void PathFind::zero()
{
  _size  .zero();
  _border.zero();
  _iteration=0;
  _map=null;
}
PathFind& PathFind::del()
{
   Free(_map);
  _active.del();
   zero(); return T;
}
PathFind& PathFind::create(Int w, Int h)
{
   if(w<0)w=_size.x;
   if(h<0)h=_size.y;
   if(_size.x!=w || _size.y!=h)
   {
      Realloc(_map, w*h, 0);
     _size.x=w;
     _size.y=h;
      REPD(y, h)
      REPD(x, w)pixel(x, y).create(x, y);
   }
   return border(0, 0, _size.x, _size.y);
}
/******************************************************************************/
UInt PathFind::pixelFlag(Int x, Int y)C
{
   return validPos(x, y) ? pixel(x, y).flag : 0;
}
Bool PathFind::pixelWalkable(Int x, Int y)C {return FlagTest(pixelFlag(x, y), PFP_WALKABLE);}
Bool PathFind::pixelTarget  (Int x, Int y)C {return FlagTest(pixelFlag(x, y), PFP_TARGET  );}
Bool PathFind::pixelStart   (Int x, Int y)C {return FlagTest(pixelFlag(x, y), PFP_START   );}

PathFind& PathFind::pixelFlag(Int x, Int y, Byte flag)
{
   if(validPos(x, y))pixel(x, y).flag=flag;
   return T;
}
PathFind& PathFind::pixelWalkable(Int x, Int y, Bool on)
{
   if(validPos(x, y))FlagSet(pixel(x, y).flag, PFP_WALKABLE, on);
   return T;
}
PathFind& PathFind::pixelTarget(Int x, Int y, Bool on)
{
   if(validPos(x, y))FlagSet(pixel(x, y).flag, PFP_TARGET, on);
   return T;
}
PathFind& PathFind::pixelStart(Int x, Int y, Bool on)
{
   if(validPos(x, y))FlagSet(pixel(x, y).flag, PFP_START, on);
   return T;
}
PathFind& PathFind::border(Int min_x, Int min_y, Int max_x, Int max_y)
{
  _border.min.x=Max(0, min_x); _border.max.x=Min(_size.x, max_x);
  _border.min.y=Max(0, min_y); _border.max.y=Min(_size.y, max_y);
   return T;
}
/******************************************************************************/
static struct MOVE
{
   VecSB2 dir;
   Byte   length;
}const Move[8]=
{
   {VecSB2( 0, 1), 5},
   {VecSB2( 0,-1), 5},
   {VecSB2( 1, 0), 5},
   {VecSB2(-1, 0), 5},

   {VecSB2( 1, 1), 7},
   {VecSB2( 1,-1), 7},
   {VecSB2(-1, 1), 7},
   {VecSB2(-1,-1), 7},
};
void PathFind::step()
{
   // update iterations (reset pixels if needed)
   if(++_iteration==0) // we've reset the counter (got back to zero again)
   {
      REP(_size.x*_size.y) // reset pixels
      {
         Pixel &pixel=_map[i];
         pixel.iteration=0; // reset iterations
      }
     _iteration=1;
   }
}
Bool PathFind::find(C VecI2 *start, C VecI2 *target, MemPtr<VecI2> path, Int max_steps, Bool diagonal, Bool reversed)
{
   const Int moves=(diagonal ? 8 : 4);

   // clear path
   path.clear();

   if(!start || validPos(*start)) // if valid start
   {
      // if already on target
      if(start && onTarget(*start, target))return true; // if there's only one 'start' point then there's no need for setting 'path'

      // no steps left
      if(!max_steps)return false;

      step();

      // setup first step
     _active.clear();
      if(start)
      {
         Pixel &pix=pixel(*start);
         pix.iteration=_iteration;
         pix.length   =0;
         pix.src      =null;
        _active.add(&pix);
      }else
      {
         for(Int y=_border.min.y; y<_border.max.y; y++)
         for(Int x=_border.min.x; x<_border.max.x; x++)
         {
            Pixel &pix=pixel(x, y); if(pix.flag&PFP_START)
            {
               VecI2 pos(x, y); if(onTarget(pos, target)){if(path)path.add(pos); return true;} // if already on target, in case where there are multiple start points ('start' is null) then add 'path' so we can know which one is on target
               pix.iteration=_iteration;
               pix.length   =0;
               pix.src      =null;
              _active.add(&pix);
            }
         }
      }

      // start searching
      UInt   found_length;
      Pixel *found=null;
      for(Int step=0; ; step++)
      {
         REPA(_active) // order is important
         {
            Pixel *p=_active[i];         // for each walker
            UInt   cur_length=p->length; // path travelled by walker
            REPD(m, moves)               // try going in each direction
            {
             C MOVE &move=Move[m];
               VecI2 v=p->xy+move.dir; if(validPos(v)) // target position
               {
                  UInt   new_length=cur_length+move.length;
                  Pixel &to        =pixel(v);

                  // zero if not used
                  if(to.iteration!=_iteration)
                  {
                     to.iteration    =_iteration;
                     to.added_in_step=-1;
                     to.length       =UINT_MAX;
                  }

                  if(new_length<to.length)
                  {
                     if(move.dir.x && move.dir.y) // diagonal move
                        if(pixel(p->xy).flag&PFP_WALKABLE) // test only if we're starting from 'walkable' pixel
                           if(!(pixel(p->xy.x, v.y).flag&PFP_WALKABLE)
                           || !(pixel(v.x, p->xy.y).flag&PFP_WALKABLE))continue;

                     if(onTarget(v, target))
                     {
                        to.length=new_length;
                        to.src   =p;
                        if(!found || new_length<found_length)
                        {
                           found       =&to;
                           found_length=new_length;
                        }
                     }else
                     if(to.flag&PFP_WALKABLE)
                     {
                        to.length=new_length;
                        to.src   =p;
                        if(to.added_in_step!=step)
                        {
                           to.added_in_step=step;
                          _active.add(&to);
                        }
                     }
                  }
               }
            }
           _active.remove(i);
         }
         if(found) // found path
         {
            if(path) // build path
            {
               if(!(found->flag&PFP_WALKABLE))found=found->src; // if the last is not walkable then remove it
               if(found)for(;;)
               {
                  if(!start)path.add(found->xy); // this          includes the starting position in the 'path'
                  Pixel *src=found->src; if(!src)break;
                  if( start)path.add(found->xy); // this does not include  the starting position in the 'path'
                  found=src;
               }
               if(!reversed)path.reverseOrder(); // 'path' is reversed by default when it's created, but if the user doesn't want that, then we need to reverse it
            }
            return true;
         }
         if(!_active.elms())return false; // no more paths left to follow
         if(!--max_steps   )return false; // ran out of steps
      }
   }
   return false;
}
/******************************************************************************/
void PathFind::getWalkableNeighbors(C VecI2 &pos, MemPtr<VecI2> pixels, Bool diagonal)
{
   pixels.clear();
   if(pixelWalkable(pos.x, pos.y))
   {
      const Int moves=(diagonal ? 8 : 4);
      step();
      pixels.add(pos); pixel(pos).iteration=_iteration;
      FREPAD(processed, pixels)
      {
         VecI2 pos=pixels[processed]; // don't use reference because 'pixels' may get modified later
         REP(moves)
         {
            VecI2 next=pos+Move[i].dir; if(validPos(next))
            {
               Pixel &pix=pixel(next); if(FlagTest(pix.flag, PFP_WALKABLE) && pix.iteration!=_iteration)
               {
                  pixels.add(next); pix.iteration=_iteration;
               }
            }
         }
      }
   }
}
/******************************************************************************/
}
/******************************************************************************/
