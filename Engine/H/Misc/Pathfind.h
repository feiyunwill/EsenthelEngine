/******************************************************************************

   Use 'PathMesh' to specify a path mesh for a single area.

   Use 'PathWorld' for pathfinding on many 'PathMesh'es.

   Use 'PathObstacle' for temporary obstacles in 'PathWorld's.

/******************************************************************************/
enum PATH_MESH_TYPE
{
   PM_OBSTACLE, // can not be crossed
   PM_GROUND  , // can     be crossed if traveller supports PMF_WALK
   PM_WATER   , // can     be crossed if traveller supports PMF_SWIM
#if EE_PRIVATE
   PM_BLOCKED=63, // covered by obstacles
#endif
};
enum PATH_MESH_FLAG
{
   PMF_WALK=0x0001,
   PMF_SWIM=0x0002,
};
/******************************************************************************/
struct PathSettings
{
   Flt areaSize  ()C {return _area_size;}   PathSettings& areaSize  (Flt size ); // get/set area size         (in meters ), 0..Inf , default=32
   Flt ctrlRadius()C {return _ctrl_r   ;}   PathSettings& ctrlRadius(Flt r    ); // get/set controller radius (in meters ), 0..Inf , default=0.33f
   Flt ctrlHeight()C {return _ctrl_h   ;}   PathSettings& ctrlHeight(Flt h    ); // get/set controller height (in meters ), 0..Inf , default=2.0f
   Flt  maxClimb ()C {return _max_climb;}   PathSettings&  maxClimb (Flt climb); // get/set max climb  height (in meters ), 0..Inf , default=0.7f
   Flt  maxSlope ()C {return _max_slope;}   PathSettings&  maxSlope (Flt slope); // get/set max slope  angle  (in radians), 0..PI_2, default=PI_4
   Flt cellSize  ()C {return _cell_size;}   PathSettings& cellSize  (Flt size ); // get/set path cell  size   (in meters ), 0..Inf , default=1.0f/3
   Flt cellHeight()C {return _cell_h   ;}   PathSettings& cellHeight(Flt h    ); // get/set path cell  height (in meters ), 0..Inf , default=0.1f

   // operations
   PathSettings& reset(); // reset to default settings

   Bool operator==(C PathSettings &path)C; // if settings are equal
   Bool operator!=(C PathSettings &path)C; // if settings are different

   // io
   Bool save(File &f)C; // save to   file, false on fail
   Bool load(File &f) ; // load from file, false on fail

   PathSettings() {reset();}

private:
   Flt _area_size, _ctrl_r, _ctrl_h, _max_climb, _max_slope, _cell_size, _cell_h;
};
/******************************************************************************/
struct PathObstacle
{
   // manage
   void del   ();                                   // delete manually                             , deleting an obstacle requires rebuilding path meshes (avoid calling this frequently as it may decrease performance)
   Bool create(C Shape &shape, C PathWorld &world); // create path obstacle from 'shape' in 'world', creating an obstacle requires rebuilding path meshes (avoid calling this frequently as it may decrease performance)

   // get / set
   Shape shape(              ); // get obstacle shape, SHAPE_NONE on fail (if doesn't exist)
   void  shape(C Shape &shape); // set obstacle shape, adjusting the shape of the obstacle requires rebuilding path meshes (avoid calling this frequently as it may decrease performance), only following shape types are supported: SHAPE_POINT, SHAPE_EDGE, SHAPE_BOX, SHAPE_BALL, SHAPE_CAPSULE, SHAPE_TUBE, SHAPE_OBOX, SHAPE_PYRAMID, attempting to set a different type of shape is equivalent to calling 'del' method

  ~PathObstacle() {del();}
   PathObstacle() {_world=null; _shape=null;}

private:
   PathWorld *_world;
   Shape     *_shape;

   NO_COPY_CONSTRUCTOR(PathObstacle);
};
/******************************************************************************/
#if EE_PRIVATE
struct RecastCompactHeightfield : rcCompactHeightfield
{
   void del();

   Bool is()C;

   void clean();

   Bool save(File &f)C;
   Bool load(File &f) ;

   void operator=(C RecastCompactHeightfield &src);

   void zero() {Zero(T);}

  ~RecastCompactHeightfield() {del ();}
   RecastCompactHeightfield() {zero();}
   RecastCompactHeightfield(C RecastCompactHeightfield &src) {zero(); T=src;}
};
const_mem_addr struct _PathMesh
{
   VecI2                    xy;
   PathWorld               *world;
   Mems<Byte>               data, obstacle_data, chf_compressed;
   RecastCompactHeightfield chf;

   // manage
   void del();

   // operations
   void link(PathWorld *world);

   void preSave(); // compress data needed for saving

   // get
   Bool is    ()C; // if has any mesh data
   Box  box   () ; // get mesh world box
   Bool getChf() ; // get chf (decompress if needed), false on fail

   // io
   Bool save(File &f)C; // save to   file, false on fail
   Bool load(File &f) ; // load from file, false on fail

 ~_PathMesh() {del();}
  _PathMesh() {xy.zero(); world=null;}

   NO_COPY_CONSTRUCTOR(_PathMesh);
};
#endif
/******************************************************************************/
struct PathMesh
{
   // manage
   void del   (); // delete manually
   Bool create(MeshBase &mesh, C VecI2 &area_xy, C PathSettings &settings); // build path mesh from source 'mesh', at 'area_xy' area coordinates using 'settings' path build settings, false on fail ("mesh.tri.flag" and "mesh.quad.flag" can be manually set to PATH_MESH_TYPE to specify the type of the face)

   // get
   Bool is()C; // if has any mesh data

   // operation
   void preSave(); // saving 'PathMesh' typically requires compressing some data, which may take some time, you can call this method at an earlier stage to compress and prepare the data so saving later will be without any delays, this method is optional and does not need to be called

   // io
   Bool save(File &f)C; // save to   file, false on fail
   Bool load(File &f) ; // load from file, false on fail

  ~PathMesh();
   PathMesh();

private:
#if EE_PRIVATE
   friend struct PathWorld;
  _PathMesh *_pm;
#else
   Ptr _pm;
#endif

   NO_COPY_CONSTRUCTOR(PathMesh);
};
/******************************************************************************/
const_mem_addr struct PathWorld // !! must be stored in constant memory address !!
{
   // manage
   void del   (             ); // delete manually
   Bool create(Flt area_size); // initialize path world manager, 'area_size'=size of a single area (in meters), false on fail

   // operations
   Bool set(PathMesh *path_mesh, C VecI2 &area_xy); // set 'path_mesh' at specified 'area_xy' area coordinates (can be null for no path mesh at that location)
#if EE_PRIVATE
   Bool _set       (_PathMesh *path_mesh, C VecI2 &area_xy, Bool set_obstacles); // set 'path_mesh' at specified 'area_xy' area coordinates (can be null for no path mesh at that location), 'set_obstacles'=if set obstacles on that mesh before setting it
   void  update    (                                                          ); // update changes in obstacles by rebuilding affected path meshes
   void  changed   (C Shape &shape                                            ); // call when 'shape' obstacle was changed in the world
   void  threadFunc();
   void  zero      ();

   VecI2     worldToArea(C Vec2  &xz     )C {return                                Floor(xz/areaSize());} // convert World Position to Area Coordinates
   VecI2     worldToArea(C Vec   &pos    )C {return                              worldToArea(pos .xz());} // convert World Position to Area Coordinates
   RectI     worldToArea(C Rect  &rect   )C {return RectI(worldToArea(rect.min), worldToArea(rect.max));} // convert World Position to Area Coordinates
   RectI     worldToArea(C Box   &box    )C {return RectI(worldToArea(box .min), worldToArea(box .max));} // convert World Position to Area Coordinates
   Box       obstacleBox(C Shape &shape  )C;
  _PathMesh* pathMesh   (C VecI2 &area_xy)C; // get PathMesh at 'area_xy' area coordinates
#endif

   // get
   Flt areaSize()C {return _area_size;} // get area size

   Bool find(C Vec &start, C Vec &end, MemPtr<Vec> path, Int max_steps=-1, UInt walkable_flags=PMF_WALK, Bool allow_partial_paths=true, C Vec &end_extents=Vec(1, 16, 1))C; // find path from 'start' to 'end' world position, 'max_steps'=maximum number of allowed steps in the result path (use <=0 for unlimited), 'walkable_flags'=faces that are walkable (PATH_MESH_FLAG), 'allow_partial_paths'=if there is no direct path from 'start' to 'end' then return partial path to the point closest to the 'end', false on fail (path not found), 'end_extents'=extents used to find nearest walkable surface to the 'end' point

   Bool nearestSurface(C Vec &pos, C Vec &extents, Vec &surface_pos, UInt walkable_flags=PMF_WALK)C; // find nearest point on path surface, 'pos'=center position of the search, 'extents'=maximum distance to search in each dimension, 'surface_pos'=output position, 'walkable_flags'=faces that are walkable (PATH_MESH_FLAG), false on fail (point not found)

   Bool nearestWall(C Vec &pos, Flt max_distance, Flt *hit_distance=null, Vec *hit_pos=null, Vec *hit_normal=null, UInt walkable_flags=PMF_WALK)C; // calculate distance to nearest wall from 'pos' position, 'max_distance'=maximum distance to perform the search, 'hit_distance'=distance to wall, 'hit_pos'=hit position, 'hit_normal'=hit normal, 'walkable_flags'=faces that are walkable (PATH_MESH_FLAG), false on fail (wall not found)

   Bool ray(C Vec &start, C Vec &end, Flt *hit_frac=null, Vec *hit_pos=null, Vec *hit_normal=null, UInt walkable_flags=PMF_WALK)C; // perform a ray test on the path surface starting from 'start' towards 'end', 'hit_frac'=fraction of the movement (0..1), 'hit_pos'=hit position, 'hit_normal'=hit normal, 'walkable_flags'=faces that are walkable (PATH_MESH_FLAG), false on fail (no hit occurred)

   // draw
   void draw(Byte surface_color_alpha=64, Flt y_offset=0, C Color &outer_edge_color=Color(0,48,64,220), C Color &inner_edge_color=Color(0,48,64,32))C; // 'y_offset'=vertical offset applied when drawing paths (can be set to positive value to draw the paths above the ground, so they will not be occluded by ground meshes)

  ~PathWorld() {del();}
   PathWorld();

#if !EE_PRIVATE
private:
#endif
   Flt              _area_size, _ctrl_r, _ctrl_h, _max_climb;
#if EE_PRIVATE
   struct Build
   {
      RecastCompactHeightfield chf;
      Mems<Shape>              shapes;
      VecI2                    xy;
      Ptr                      user;
   };
   struct Built
   {
      Mems<Byte> data;
      VecI2      xy;
      Ptr        user;
   };

   dtNavMesh       *_mesh;
   dtNavMeshQuery  *_query;
   dtQueryFilter   *_filter;
   Memc<_PathMesh*> _changed;
   Memc<Build>      _build;
   Memc<Built>      _built;
#else
   Ptr              _mesh, _query, _filter;
  _Memc             _changed, _build, _built;
#endif
   Memx<Shape>      _obstacles;
   Thread           _thread;
   SyncEvent        _event;
   SyncLock         _lock;

   NO_COPY_CONSTRUCTOR(PathWorld);
};
/******************************************************************************/
