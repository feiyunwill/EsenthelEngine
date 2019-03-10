/******************************************************************************

   Use 'PathFind' for 2D map path finding.

/******************************************************************************/
enum PATH_FIND_PIXEL_FLAG // PathFind Pixel Flags
{
   PFP_WALKABLE=0x1, // walkable pixel
   PFP_TARGET  =0x2, // destination target
   PFP_START   =0x4, // beginning
};
/******************************************************************************/
struct PathFind // 2D Map Path Finder
{
   // manage
   PathFind& create(Int w, Int h); // create and set map size to 'w*h'

   // get
   Int w()C {return _size.x;} // get map width
   Int h()C {return _size.y;} // get map height

   Bool validPos(Int x, Int y)C {return x<_border.max.x && y<_border.max.y && x>=_border.min.x && y>=_border.min.y;} // if selected position is valid (lies within allowed border)

#if EE_PRIVATE
   struct Pixel
   {
      Byte   flag;
      Int    added_in_step;
      UInt   length, iteration;
      VecI2  xy;
      Pixel *src;

      void create(Int x, Int y);
   };

   Pixel& pixel   (Int x, Int y)  {return _map[x + y*_size.x];}
 C Pixel& pixel   (Int x, Int y)C {return _map[x + y*_size.x];}
   Pixel& pixel   (C VecI2 &v  )  {return  pixel   (v.x, v.y);}
 C Pixel& pixel   (C VecI2 &v  )C {return  pixel   (v.x, v.y);}
   Bool   validPos(C VecI2 &v  )  {return  validPos(v.x, v.y);}
   Bool   onTarget(C VecI2 &v, C VecI2 *target)C {return target ? v==*target : FlagTest(pixel(v).flag, PFP_TARGET);} // !! 'v' is assumed to be 'validPos' !!
   void   step    ();
   void   zero    ();
#endif

   // set
   UInt pixelFlag    (Int x, Int y)C;   PathFind& pixelFlag    (Int x, Int y, Byte flag); // get/set    map pixel flag PATH_FIND_PIXEL_FLAG
   Bool pixelWalkable(Int x, Int y)C;   PathFind& pixelWalkable(Int x, Int y, Bool on  ); // get/set if map pixel is   walkable
   Bool pixelTarget  (Int x, Int y)C;   PathFind& pixelTarget  (Int x, Int y, Bool on  ); // get/set if map pixel is a destination target by marking it with PFP_TARGET flag
   Bool pixelStart   (Int x, Int y)C;   PathFind& pixelStart   (Int x, Int y, Bool on  ); // get/set if map pixel is a beginning          by marking it with PFP_START  flag

   PathFind& border(Int min_x, Int min_y, Int max_x, Int max_y); // specify borders outside of which moving is forbidden, this allows to specify the walking rectangle area, default=(0, 0, x(), y())

   // operations
   Bool find(C VecI2 *start, C VecI2 *target, MemPtr<VecI2> path, Int max_steps=-1, Bool diagonal=true, Bool reversed=false); // find path from 'start' to 'target' (if 'start' is null then all pixels with PFP_START are treated as the starting points, if 'target' is null then all pixels with PFP_TARGET are treated as target points), 'max_steps'=maximum allowed steps (-1=unlimited) allows to limit the maximum number of pathfind iterations, 'diagonal'=if allow diagonal movements, 'reversed'=if reverse the path, false on fail

   void getWalkableNeighbors(C VecI2 &pos, MemPtr<VecI2> pixels, Bool diagonal=true); // get a list of all neighbor walkable pixels, 'diagonal'=if allow diagonal movements

   PathFind&   del(); // delete manually
  ~PathFind() {del();}
   PathFind();

private:
   VecI2     _size;
   RectI     _border;
   UInt      _iteration;
#if EE_PRIVATE
   Pixel       *_map;
   Memc<Pixel*> _active;
#else
   Ptr       _map;
   Memc<Ptr> _active;
#endif

   NO_COPY_CONSTRUCTOR(PathFind);
};
/******************************************************************************/
