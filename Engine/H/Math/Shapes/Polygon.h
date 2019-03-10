/******************************************************************************/
#if EE_PRIVATE
struct Poly // Polygon
{
   struct Vtx // polygon vertex
   {
      Int index, // point index
          edge ; // edge  index
      Flt angle; // angle

      void set(Int index          ) {T.index=index;}
      void set(Int index, Int edge) {T.index=index; T.edge=edge;}
   };

   Byte      angle; // if angles should be calculated
   Int       id   ; // id number
   Vec      *pos  ; // pointer to vertex positions
   Meml<Vtx> vtx  ; // list of vertexes

   // manage
   Poly& del   (                  ); // delete
   Poly& create(Vec *pos, Int id=0); // create

   // get / set
   Bool infinite() ; // if infinite
   Flt  length2D()C; // get 2D length
   Flt  length3D()C; // get 3D length
   void setAngle() ; // recalculate angles of vertexes

   // operations
   void    addVtx(Int index          ) {vtx.New().set(index      );} // add    vertex
   void    addVtx(Int index, Int edge) {vtx.New().set(index, edge);} // add    vertex and set edge
   void removeVtx(MemlNode *v                          );            // remove vertex
   void link     (Poly &poly, MemlNode *i, MemlNode *pi);            // link with vertexes from poly, 'i'=connected vertex from self, 'pi'= connected vertex from 'poly', 'poly' remains unmodified

   // draw
   void draw2D(C Color &color);
   void draw3D(C Color &color); // this relies on active object matrix which can be set using 'SetMatrix' function

  ~Poly() {del();}
   Poly();

private:
   Byte _infinite;
};
#endif
/******************************************************************************/
void CreateConvex2D  (MemPtr<Vec2 > poly, C Vec2  *point, Int points); // create convex 'poly' from points xy coordinates ('point'=array of points, 'points'=number of points)
void CreateConvex2D  (MemPtr<VecD2> poly, C VecD2 *point, Int points); // create convex 'poly' from points xy coordinates ('point'=array of points, 'points'=number of points)
void CreateConvex2Dxz(MemPtr<Vec2 > poly, C Vec   *point, Int points); // create convex 'poly' from points xz coordinates ('point'=array of points, 'points'=number of points)
void CreateConvex2Dxz(MemPtr<VecD2> poly, C VecD  *point, Int points); // create convex 'poly' from points xz coordinates ('point'=array of points, 'points'=number of points)

void Triangulate(        C MemPtr<Vec    >   &poly , MeshBase &mesh,                                      Bool convex=false                         ); // triangulate         'poly' to 'mesh', 'mesh' will be deleted at start                                                                                                                                   , 'convex'=if 'polys' are known to be convex (this will speed up the process)
void Triangulate(        C MemPtr<VtxFull>   &poly , MeshBase &mesh, UInt flag_and,                       Bool convex=false                         ); // triangulate         'poly' to 'mesh', 'mesh' will be deleted at start, 'flag_and'=elements to include in 'mesh' creation (MESH_BASE_FLAG)                                                               , 'convex'=if 'polys' are known to be convex (this will speed up the process)
void Triangulate(C MemPtr< Memc  <Vec    > > &polys, MeshBase &mesh               , Flt weld_pos_eps=EPS, Bool convex=false, C Byte *poly_flags=null); // triangulate set of 'polys' to 'mesh', 'mesh' will be deleted at start                                                                    , 'weld_pos_eps'=epsilon used for final vertex position welding, 'convex'=if 'polys' are known to be convex (this will speed up the process), 'poly_flags'=if not null then mesh faces will have their flags set according to given pointer (its length must be as long as number of 'polys')
void Triangulate(C MemPtr< Memc  <VtxFull> > &polys, MeshBase &mesh, UInt flag_and, Flt weld_pos_eps=EPS, Bool convex=false, C Byte *poly_flags=null); // triangulate set of 'polys' to 'mesh', 'mesh' will be deleted at start, 'flag_and'=elements to include in 'mesh' creation (MESH_BASE_FLAG), 'weld_pos_eps'=epsilon used for final vertex position welding, 'convex'=if 'polys' are known to be convex (this will speed up the process), 'poly_flags'=if not null then mesh faces will have their flags set according to given pointer (its length must be as long as number of 'polys')

void ClipPoly (C MemPtr<Vec    > &poly, C Plane &plane, MemPtr<Vec    > output                                          ); // clip  'poly' according to 'plane' and store result  in 'output'  , 'poly' and 'output'   must point to different containers
void ClipPoly (C MemPtr<VtxFull> &poly, C Plane &plane, MemPtr<VtxFull> output                                          ); // clip  'poly' according to 'plane' and store result  in 'output'  , 'poly' and 'output'   must point to different containers
void SplitPoly(C MemPtr<Vec    > &poly, C Plane &plane, MemPtr<Vec    > output_positive, MemPtr<Vec    > output_negative); // split 'poly' according to 'plane' and store results in 'output_*', 'poly' and 'output_*' must point to different containers
void SplitPoly(C MemPtr<VtxFull> &poly, C Plane &plane, MemPtr<VtxFull> output_positive, MemPtr<VtxFull> output_negative); // split 'poly' according to 'plane' and store results in 'output_*', 'poly' and 'output_*' must point to different containers

void DrawPoly2D(C MemPtr<Vec> &poly, C Color &edge_color=WHITE, C Color &vtx_color=TRANSPARENT); // draw 'poly'
void DrawPoly  (C MemPtr<Vec> &poly, C Color &edge_color=WHITE, C Color &vtx_color=TRANSPARENT); // draw 'poly', this relies on active object matrix which can be set using 'SetMatrix' function
/******************************************************************************/
