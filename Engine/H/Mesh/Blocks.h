/******************************************************************************/
struct BlocksOcclusion // helper class used for Blocks Ambient Occlusion calculation
{
   enum COMBINE_MODE : Byte // Combine Mode
   {
      ADD,
      MAX,
   };

   // get
   Int  steps    ()C {return _steps   [0];} // get number of steps    set during creation
   Int  steps1   ()C {return _steps   [1];} // get number of steps    set during creation
   Flt  strength ()C {return _strength[0];} // get occlusion strength set during creation
   Flt  strength1()C {return _strength[1];} // get occlusion strength set during creation
   Bool angles   ()C {return _angles     ;}
   Int  maxSteps ()C {return Max(_steps[0], _steps[1]);} // get max number of steps

   COMBINE_MODE aoCombineMode()C {return _aocm;} // get dual ambient occlusion combine mode

   // manage
   void create(Int steps=2, Flt strength=1.0f, Int steps_1=0, Flt strength_1=0.0f, COMBINE_MODE ao_combine_mode=ADD, Bool angles=false); // 'steps'=number of block steps used during occlusion calculation, bigger number will increase the quality however it will also reduce performance, 'strength'=occlusion strength 0..1 (0=no occlusion, 1=full occlusion), use 'steps_1 and strength_1 ao_combine_mode' for a secondary ambient occlusion effect (dual ambient occlusion), 'angles'=if include angle between light and surface normal when computing occlusion

   // operations, each of these methods recreate the entire 'BlocksOcclusion' by calling 'create', with just one parameter changed
   BlocksOcclusion& steps        (Int          steps   );
   BlocksOcclusion& steps1       (Int          steps   );
   BlocksOcclusion& strength     (Flt          strength);
   BlocksOcclusion& strength1    (Flt          strength);
   BlocksOcclusion& aoCombineMode(COMBINE_MODE aocm    );
   BlocksOcclusion& angles       (Bool         on      );

   BlocksOcclusion();

#if !EE_PRIVATE
private:
#endif
   struct Node
   {
      Byte       dir;
      VecI       pos;
      UInt       fraction_axis[4];
      Flt        fraction     [2];
      Memc<Node> nodes;

   #if EE_PRIVATE
      Node& init(C VecI &pos);
   #endif
   };

   Bool         _angles;
   COMBINE_MODE _aocm;
   Int          _steps   [2];
   Flt          _strength[2];
   Memc<Node>   _nodes;
};
/******************************************************************************/
struct Blocks
{
   struct Light
   {
      Ball ball; // world space range and position (not 'Blocks' local position)
      Vec color; // light color (0, 0, 0) .. (1, 1, 1)

      Light& set(C Ball &ball, C Vec &color=1) {T.ball=ball; T.color=color; return T;}
   };

   // manage
   Blocks& del   (                                  ); // delete manually
   Blocks& create(Int resolution, Int sub_division=1); // create Blocks capable of storing 'resolution' number of blocks in X Z dimensions, Y dimension will have no limit, 'sub_division'=sub division for mesh faces, 1..Inf

   // get / set
   Bool is         ()C {return _levels.elms()>0;}                                   // if has any data
   Int  resolution ()C {return _resolution     ;}                                   // get resolution used at Blocks creation
   Int  subDivision()C {return _sub_division   ;}   Blocks& subDivision(Int steps); // get/set mesh sub division, changing the 'subDivision' does not automatically rebuild the mesh, you should call 'setMesh' afterwards

   Bool        hasBlock(C VecI &pos                                                          )C; // if there is a block     at 'pos' local coordinates
   MaterialPtr material(C VecI &pos                                                          )C; // get     block material  at 'pos' local coordinates (this will return the 'top' material of the block)
   void        get     (C VecI &pos, MaterialPtr &top, MaterialPtr &side, MaterialPtr &bottom)C; // get all block materials at 'pos' local coordinates
#if EE_PRIVATE
   Bool        hasBlock(C VecI &pos, Int min_level_i)C;
   Bool set(Int x, Int y, Int z, Byte b);
#endif

   Bool set(Int x, Int y, Int z, C MaterialPtr &material                                       ); // set block to 'material'                , true is returned if any change was made during this operation, this does not automatically rebuild the mesh or the physical body, after calling this method you should call 'setMesh' and 'setPhysBody'
   Bool set(Int x, Int y, Int z, C MaterialPtr &top, C MaterialPtr &side, C MaterialPtr &bottom); // set block material to 'top side bottom', true is returned if any change was made during this operation, this does not automatically rebuild the mesh or the physical body, after calling this method you should call 'setMesh' and 'setPhysBody'

   // mesh
   Blocks& setMesh(Flt tex_scale, C BlocksOcclusion *occl=null, C BoxI *local_box=null, C VecI2 &blocks_pos=0, C BlocksMap *map=null, C MemPtr<Light> &lights=null, Bool optimize=false, Flt max_face_length=-1); // set Blocks mesh, 'tex_scale'=texture coordinates scale, 'occl'=Blocks occlusion object used for ambient occlusion calculation (use null for no AO), 'local_box'=if specified then the mesh will only be updated in specified local coordinates (use null for setting entire mesh), 'blocks_pos'=position of this Blocks object inside the 'BlocksMap' (based on this position neighbors will be obtained from the map), 'map'=map of all Blocks objects (you should pass a pointer to your custom class that extends from 'BlocksMap' to provide custom 'findBlocks' method which will be used for finding neighbors to this Blocks object), 'lights'=array of world space lights, 'optimize'=if automatically optimize the mesh by welding coplanar faces which will result in faster rendering ('max_face_length'=max allowed face length, -1=no limit)

   void setShader() {_mesh.setShader();} // reset mesh shader

   // physics
   Blocks& setPhysBody(C Matrix &matrix, C BoxI *local_box=null, Bool create_actor=false); // set Blocks physical body, 'matrix'=matrix of this 'Blocks' object, 'local_box'=if specified then the physical body will only be updated in specified local coordinates (use null for setting entire physical body), 'create_actor'=if automatically re-create the actor after updating the physical body
   Blocks&    delActor(); // delete blocks actor
   Blocks& createActor(); // create blocks actor from the physical body ('setPhysBody' should be called earlier)

   // operations
   Bool raycast(C Vec &start, C Vec &move, Vec *hit_pos=null, Vec *hit_normal=null, C Matrix *matrix=null)C; // raycast from 'start' local position along 'move' vector, returns true if collision occurred and sets 'hit_pos' to its position and 'hit_normal' to its normal vector, 'matrix'=matrix of this 'Blocks' object

   // clean
   Blocks& cleanLevels   (); // clean Blocks from unused Y levels
   Blocks& cleanMaterials(); // clean Blocks from unused materials

   // draw
   void draw      (C MatrixM &matrix)C {if(Frustum(_mesh, matrix))_mesh.draw      (matrix);} // 'matrix'=matrix of this 'Blocks' object
   void drawShadow(C MatrixM &matrix)C {if(Frustum(_mesh, matrix))_mesh.drawShadow(matrix);} // 'matrix'=matrix of this 'Blocks' object

   // io
   Bool save(File &f, Bool include_mesh_and_phys_body, CChar *path=null)C; // save to   file, 'include_mesh_and_phys_body' if include mesh and physical body in the file, when enabled you don't need to build the mesh and physical body after load, 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail
   Bool load(File &f                                 , CChar *path=null) ; // load from file                                                                                                                                                        , 'path'=path at which resource is located (this is needed so that the sub-resources can be accessed with relative path), false on fail

#if EE_PRIVATE
   Bool cleanMtrlCombos(Bool is[256], Byte remap[256]);
   void zero();
#endif
   Blocks();

#if !EE_PRIVATE
private:
#endif
   struct Level
   {
      Int      y;
      Byte   *_map;
      PhysPart phys;
      Actor    actor;

   #if EE_PRIVATE
      void init(Int resolution       ) {AllocZero(Free(_map), resolution*resolution);}
      void init(Int resolution, Int y) {T.y=y; init(resolution);}

      Byte& map(Int x, Int z, Int resolution)  {return _map[x+z*resolution];}
      Byte  map(Int x, Int z, Int resolution)C {return ConstCast(T).map(x, z, resolution);}

      Bool wallX      (Int x, Int z, Int res               )C {return (InRange(x-1, res) ? map(x-1, z  , res)!=0 : false) != (InRange(x, res) ? map(x, z, res)!=0 : false);}
      Bool wallZ      (Int x, Int z, Int res               )C {return (InRange(z-1, res) ? map(x  , z-1, res)!=0 : false) != (InRange(z, res) ? map(x, z, res)!=0 : false);}
      Bool wallBack   (Int x, Int z, Int res               )C {return map(x, z, res) && (InRange(z-1, res) ? !      map(x, z-1, res) : true);}
      Bool wallForward(Int x, Int z, Int res               )C {return map(x, z, res) && (InRange(z+1, res) ? !      map(x, z+1, res) : true);}
      Bool wallLeft   (Int x, Int z, Int res               )C {return map(x, z, res) && (InRange(x-1, res) ? !      map(x-1, z, res) : true);}
      Bool wallRight  (Int x, Int z, Int res               )C {return map(x, z, res) && (InRange(x+1, res) ? !      map(x+1, z, res) : true);}
      Bool wallBottom (Int x, Int z, Int res, C Level *down)C {return map(x, z, res) && (down              ? !down->map(x  , z, res) : true);}
      Bool wallTop    (Int x, Int z, Int res, C Level *up  )C {return map(x, z, res) && (up                ? !up  ->map(x  , z, res) : true);}

      void createPhysBody(C Matrix &matrix, Int res, C Level *down, C Level *up);
      void createActor   (                                                     ) {actor.create(phys, 0);}

      Bool save(File &f, Int resolution, Bool include_mesh_and_phys_body)C;
      Bool load(File &f, Int resolution, Bool include_mesh_and_phys_body) ;
   #endif

      void     del();
     ~Level() {del();}
      Level() {y=0; _map=null;}
      NO_COPY_CONSTRUCTOR(Level);
   };
   struct MtrlCombo
   {
      Byte top, side, bottom; // material indexes in '_materials'

   #if EE_PRIVATE
      MtrlCombo& set(Byte top, Byte side, Byte bottom) {T.top=top; T.side=side; T.bottom=bottom; return T;}
   #endif
   };

   Int             _resolution, _sub_division;
   MaterialPalette _materials;
   Memc<MtrlCombo> _mtrl_combos;
   Memc<Level>     _levels;
   Mesh            _mesh;

#if EE_PRIVATE
   struct LightEx : Light
   {
      Vec world_pos;
   };
   struct Neighbors
   {
    C Blocks *l, *r, *b, *f, *lb, *lf, *rb, *rf;

      void clear();
      void set  (C Blocks *l, C Blocks *r, C Blocks *b, C Blocks *f, C Blocks *lb, C Blocks *lf, C Blocks *rb, C Blocks *rf, Int resolution);

      Neighbors(C Blocks *l, C Blocks *r, C Blocks *b, C Blocks *f, C Blocks *lb, C Blocks *lf, C Blocks *rb, C Blocks *rf, Int resolution);
      Neighbors(C BlocksMap *map, Int x, Int y, Int resolution);
   };
   struct LevelBrightness // precalculated brightness of a blocks level
   {
      struct Point
      {
         Color brightness[6]; //    brightness of each direction            , DIR_ENUM 
         Byte  computed_dir ; // if brightness of      direction is computed, DIR_FLAG
         Int   computed_y   ; // y coordinates of the computed brightness

         void clear() {computed_dir=0;}
      };

      Int                  size, min_level_i;
      Memt<Point>          points;
    C Blocks              &blocks;
    C Neighbors           &neighbors;
    C BlocksOcclusion     *occl;
    C BlocksMap           *map;
    C VecI2               &blocks_pos;
    C MemtN<LightEx, 256> &lights;

      LevelBrightness(C Blocks &blocks, C Neighbors &neighbors, C BlocksOcclusion *occl, C BlocksMap *map, C VecI2 &blocks_pos, C MemtN<LightEx, 256> &lights);

      // operations
      void clear();
      void clear(Int min_x, Int min_z, Int max_x, Int max_z);
      LevelBrightness& setLevelI(Int level_i);

      // get
      Point& point     (Int x,        Int z) {return points[x+z*size];}
    C Color& brightness(Int x, Int y, Int z, DIR_ENUM dir);
   };

   struct Part
   {
      struct Vtx
      {
         Vec   pos;
         Vec2  tex;
         Color col;

         void set(C Vec &pos, C Vec2 &tex, C Color &col) {T.pos=pos; T.tex=tex; T.col=col;}
      };
      Memc<Vtx  > vtxs ;
      Memc<VecI4> quads;

      Bool is()C {return vtxs.elms()!=0;}

      void create(MeshBase &base);
   };

   static Int CompareLevel(C Level &level, C Int &y) {return Compare(level.y, y);}

   Int    findLevelI(Int y           )C;
   Level* findLevel (Int y, Int  from);
 C Level* findLevel (Int y, Int  from)C {return ConstCast(T).findLevel(y, from);}
   Level* findLevel (Int y           );
 C Level* findLevel (Int y           )C {return ConstCast(T).findLevel(y);}
   Level*   toLevel (Int y, Int &last);
 C Level*   toLevel (Int y, Int &last)C {return ConstCast(T).toLevel(y, last);}
   Level&  getLevel (Int y           );

   UInt occlusion(C BlocksOcclusion::Node &node, C VecI &pos, AXIS_TYPE axis, Int min_level_i, C Neighbors &neighbors)C;

   Byte brightness(C BlocksOcclusion *occl, Int x, Int y, Int z, AXIS_TYPE axis, UInt dir_flag, Int min_level_i, C Neighbors &neighbors)C;

   void buildLevel(Flt tex_scale, MemPtrN<Part, 256> parts, Int min_x, Int min_z, Int max_x, Int max_z, Int level_i, C Neighbors &neighbors, LevelBrightness (&lb)[2]);
#endif
};
/******************************************************************************/
struct BlocksMap // extend this class to provide your own 'findBlocks' method
{
   // manage
   void create(Int resolution, C Matrix &base_matrix); // 'resolution'=resolution of all 'Blocks' in this map, 'base_matrix'=transformation matrix of Blocks at (0, 0) coordinates

   // get
   Int    resolution()C {return _resolution ;} // get resolution
   Matrix baseMatrix()C {return _base_matrix;} // get base matrix
   Matrix matrix    (Int x, Int y)C; // get transformation matrix of 'Blocks' at (x, y) coordinates, it will be calculated based on 'resolution' and 'base_matrix'

   // operations
   Bool raycast(C Vec &start, C Vec &move, Vec *hit_pos=null, Vec *hit_normal=null)C; // raycast from 'start' local position along 'move' vector, returns true if collision occurred and sets 'hit_pos' to its position and 'hit_normal' to its normal vector

   Vec light(C Vec &pos, C BlocksOcclusion *occl=null, C MemPtr<Blocks::Light> &lights=null)C; // calculate light at 'pos' position, 'lights'=lights present in the scene

   // custom
   virtual Blocks* findBlocks(Int x, Int y)C {return null;} // override this method and return pointer to 'Blocks' at specified location

   BlocksMap();

private:
   Int    _resolution;
   Matrix _base_matrix;
};
/******************************************************************************/
