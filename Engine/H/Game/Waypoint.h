/******************************************************************************

   Use 'Waypoint' to handle helper world positions, paths management.
      You can use it to:
         -specify custom positions
         -specify custom paths

/******************************************************************************/
namespace Game{
/******************************************************************************/
struct Waypoint
{
   enum LOOP_MODE : Byte
   {
      SINGLE   , // Waypoint is a single    path (for example 0, 1, 2             )
      LOOP     , // Waypoint is a looped    path (for example 0, 1, 2, 0, 1, 2, ..)
      PING_PONG, // Waypoint is a ping-pong path (for example 0, 1, 2, 1, 0, 1, ..)
   };
   struct Point
   {
      Vec pos         ; // point position
      Flt total_length; // total length from starting position
   };

   LOOP_MODE   loop_mode; // looping mode, default=SINGLE
   Mems<Point> points;

   // get
   Flt length(                        )C; // get                 total length
   Vec pos   (Flt x, Bool smooth=false)C; // get position at 'x' total length, 'smooth' if use additional spline based smoothing

   // operations
   void      New               (       C Vec &pos); // create a new point at the end at 'pos' position
   void      New               (Int i, C Vec &pos); // create a new point at    i-th at 'pos' position
   Waypoint& remove            (Int i            ); // remove i-th point
   Waypoint& rol               (                 ); // rotate left  order of points
   Waypoint& ror               (                 ); // rotate right order of points
   Waypoint& reverse           (                 ); // reverse      order of points
   Waypoint& updateTotalLengths(                 ); // update 'total_length' of points according to their positions

   // draw
   void draw(C Color &point_color=WHITE, C Color &edge_color=WHITE, Flt point_radius=0.5f, Int edge_steps=1)C; // this can be optionally called outside of Render function, 'edge_steps'=how many steps apply for edge smoothing (<=1 applies no smoothing)

   // io
   Bool save(C Str  &name)C; // save, false on fail
   Bool load(C Str  &name) ; // load, false on fail
   Bool save(  File &f   )C; // save, false on fail
   Bool load(  File &f   ) ; // load, false on fail
#if EE_PRIVATE
   Bool saveData(File &f)C; // save, false on fail
   Bool loadData(File &f) ; // load, false on fail

   void zero();
#endif

   Waypoint& del(); // delete manually
   Waypoint();
};
/******************************************************************************/
extern Cache<Waypoint> Waypoints;
/******************************************************************************/
void DrawWaypoint(C Vec &pos, C Color &color=WHITE, Flt radius=0.5f);
/******************************************************************************/
} // namespace
/******************************************************************************/
inline Int Elms(C Game::Waypoint &waypoint) {return waypoint.points.elms();}
/******************************************************************************/
