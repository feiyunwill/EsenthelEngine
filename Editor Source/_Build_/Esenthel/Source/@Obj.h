/******************************************************************************/
/******************************************************************************/
class Obj : ObjData
{
   static int ComparePtr(Obj*C &a, Obj*C &b);

   Area         *area;
   bool          selected, highlighted,
                 invalid_refs,
                 visible; // this should be reflection of the 'World.obj_visible'
   EDIT_OBJ_TYPE edit_type;
   MeshPtr       mesh, mesh_proper; // 'mesh_proper' is set to proper MeshPtr to avoid reloading/unloading meshes in case they are empty because 'mesh' will be forced to 'WorldEdit.dummy_mesh' if empty
   int           mesh_variation;
   PhysBodyPtr   phys;
   ImagePtr      icon;
   Particles     particles;
   Vec           light_col;
   flt           light_angle, light_falloff;
   bool          light_cast_shadows;

  ~Obj();

   // get
   bool setInvalidRefs(); 
   bool    invalidRefs()C;
   cchar8* variationName()C;
   Matrix drawMatrix();
   Vec pos();
   Vec center();
   bool getBox(Box &box);
   bool onGround();
   bool embedded();
   void getCol(Color &col_lit, Color&col_shape);
   PhysPath physPath()C;                      

   // operations
   void attach(WorldData &world, Area *a=null);
   void detach();

   void setUpdatability();
   void setVisibility();
   void removeChanged();
   void remove();
   void create(C Obj &src);
   bool sync(C ObjData &src, cchar *edit_path);
   bool undo(C ObjData &src, cchar *edit_path);

   Obj& setChangedEmbed(C VecI2 *area_xy=null); // must be called after 'changedObj' and before 'setChangedPaths'
   Obj& setChangedPaths(); // must be called after 'setChangedEmbed'
   Obj& setChanged(bool matrix=false); // this needs to be called after making all changes
   void setUndo(cptr type=WorldUndo::ObjType, bool as_new=false);
   void moveTo(C Vec &pos, cptr undo_type=WorldUndo::ObjType);
   void moveTo(C Matrix3 &orn, cptr undo_type=WorldUndo::ObjType);
   void moveTo(C Matrix &matrix, cptr undo_type=WorldUndo::ObjType);

   void scaleBy (flt f);
   void scaleXBy(flt f);
   void scaleYBy(flt f);
   void scaleZBy(flt f);

   void randomRot();
   void  resetRot();

   void alignGrid();
   void alignTerrain(bool force, cptr undo_type=WorldUndo::ObjType);
   void alignNormal(int axis=-1, flt blend=1);

   // update
   void update();

   // draw
   bool skipDraw();
   void draw();
   void drawShadow();
   void drawParticle();
   void drawSelected();
   void drawHelper(bool box, bool phys);

   // io
   void setDraw();
   void setMeshVariation();
   void setMeshPhys();
   bool save(File &f);
   bool load(File &f); // 'setMeshPhys' doesn't need to be called because it will be called in 'attach->removeChanged'

public:
   Obj();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
