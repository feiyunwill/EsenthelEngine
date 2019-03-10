/******************************************************************************/
/******************************************************************************/
class RayTest // first all areas are added, then they are sorted depending on distance to camera, to test closer objects first
{
   class Area
   {
      ::Area *area;
      int    dist;

      Area(::Area &area, int dist);

public:
   Area();
   };
   static int Compare(C Area &a, C Area &b);

   bool       hit, on_heightmap, heightmaps, objects;
   flt        frac;
   VecI2      center;
   Vec        from, move;
   Obj       *obj;
   SyncLock   lock;
   Memc<Area> areas; // areas to process
   Memc<Obj*> objs2D; // 2d objects

   void clear();
   void init(C VecI2 &center, C Vec &from, C Vec &move, bool heightmaps, bool objects);

   static void ListAreas(Cell< ::Area> &cell, RayTest &ray_test); // add areas sorted by distance to center
   static void Test( Area &area, RayTest &rt, int thread_index); 
          void test(::Area &area); // !! this function should be multi-threaded safe !!

   bool test()C;

   bool possibleHit(flt frac)C;                        
   void      setHit(flt frac, bool heightmap, Obj*obj);
   void add2DObj(Obj *obj);                          

   bool process();

public:
   RayTest();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
