/******************************************************************************/
/******************************************************************************/
class Area : BlendObject
{
   bool            loaded, changed, invalid_refs;
   Heightmap2     *hm;
   MeshGroup       obj_mesh; // this should not be drawn, because objects are drawn from 'objs' container, 'obj_mesh' is just a copy in the memory for load/save
   PhysBody        obj_phys;
   Memc<WaterMesh> waters; // this should not be drawn, because waters are drawn from 'lakes' 'rivers' containers, 'water' is just a copy in the memory for load/save
   PathMesh        path;
   VecI2           xy;
   Area          *_l, *_r, *_b, *_f, *_lb, *_lf, *_rb, *_rf; // neighbors
   Memc<Obj*>      objs;
   WorldData      *world;
   AreaVer        *ver;
   flt             validate_refs_time;

  ~Area();
   Area(C VecI2 &xy, ptr grid_user);

   Heightmap2* hm_l (); // get left        neighbor heightmap
   Heightmap2* hm_r (); // get right       neighbor heightmap
   Heightmap2* hm_b (); // get back        neighbor heightmap
   Heightmap2* hm_f (); // get front       neighbor heightmap
   Heightmap2* hm_lb(); // get left  back  neighbor heightmap
   Heightmap2* hm_lf(); // get left  front neighbor heightmap
   Heightmap2* hm_rb(); // get right back  neighbor heightmap
   Heightmap2* hm_rf(); // get right front neighbor heightmap

   bool hasSelectedObj()C;     
   Obj* findobj(C UID &obj_id);

   Vec2 pos2D()C;

   flt hmHeight(C Vec  &pos, bool smooth=true)C;
   flt hmHeight(C Vec2 &xz , bool smooth=true)C;

   Vec hmNormal(C Vec  &pos)C;
   Vec hmNormal(C Vec2 &xz )C;

   Vec hmNormalAvg(C Vec  &pos, flt r)C;
   Vec hmNormalAvg(C Vec2 &xz , flt r)C;

   Vec hmNormalAvg(C Matrix &matrix, C Box &box)C;

   Vec hmNormalNeighbor(int x, int y);
   bool hmColor(C Vec2 &xz, Vec &color)C;
   MaterialPtr hmMtrl(C Vec2 &xz, C MaterialPtr &cur_mtrl=null)C;

   // operations
      // !! these functions must be multi-threaded SAFE !!
      AreaVer*          getVer(                      );  // ThreadSafeMap.get is multi-thread safe
    C AreaVer*          getVer(                      )C;
      void setTerrainUndo     (                      ); 
      void setServer          (                      ); 
      void setChanged         (                      ); 
      void setChangedHmRemoved(C TimeStamp *time=null);  // having 'time' means that this was called by the server, in that case don't call the undo
      void setChangedHeight   (bool skip_mesh=true   ); 
      void setChangedMtrl     (                      ); 
      void setChangedColor    (                      ); 
      void setChangedObj      (                      ); 
      void setChangedObj      (Obj &obj              );  // send only that object to the server (and not whole area)

   void setShader();
 //void hmReset() {if(hm)hm.mesh.del();} // force rebuilding of the heightmap mesh during update process
   bool hmDel(C TimeStamp *time=null);
   bool hmCreate(bool align_height_to_neighbors);
   void hmBuild();

   bool invalidRefs()C;
   void        validateRefs();
   void delayedValidateRefs(); // update after 2 seconds
   void  updateValidateRefs();

   void undo(C WorldChange::Terrain &undo, bool skip_mesh=false);

   // draw
   void draw();
   void drawShadow();
   virtual void drawBlend()override;
   void draw2D();

   // io
   bool saveEdit(C Str &name);
   void loadEdit(C Str &name);
   bool saveGame(C Str &name);
   void loadGame(C Str &name);
   void load();
   static void Flush(Cell<Area> &cell, ptr);
          void flush();
   void unload();

public:
   Area();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
