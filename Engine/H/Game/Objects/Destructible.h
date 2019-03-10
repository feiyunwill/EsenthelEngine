/******************************************************************************

   Use 'Destructible' class for destructible objects, which can be destroyed into pieces.

/******************************************************************************/
namespace Game{
/******************************************************************************/
STRUCT(Destructible , Obj) // Game Destructible Object
//{
   enum MODE : Byte // each destructible object can be in different mode
   {
      STATIC   , // this object is                 the whole DestructMesh, it uses 1 Mesh and 1    static   Actor  , this object works as typical Static object, it cannot be broken into pieces until it's manually converted to different mode, it uses 1 Mesh and 1 Actor so the cost of displaying and physics processing is low
      BREAKABLE, // this object is                 the whole DestructMesh, it uses 1 Mesh and many sleeping Actor's, this object                                    can    be broken into pieces automatically, once movement is detected on at least one of the actors, the object is automatically converted into many objects of 'PIECE' mode. This mode uses 1 Mesh and many Actors, so the cost of displaying is low, but physics processing is higher
      PIECE    , // this object is only 1 piece of the whole DestructMesh, it uses 1 Mesh and 1             Actor  , but since there are usually many pieces, there are many meshes and actors in total, so the cost of displaying and physics processing is highest
   };
   struct Joint
   {
      Reference<Destructible> destr;
      EE::Joint               joint;

      Bool save(File &f)C;
      Bool load(File &f) ;
   };

   MODE          mode          ; // destructible object mode
   Int           piece_index   ; // index of piece in 'destruct_mesh', this is valid only in 'PIECE' mode, in other modes this is equal to -1
   Flt           scale         ; // scale
   ObjectPtr     base          ; // base object
   MeshPtr       mesh          ; // mesh
   Int           mesh_variation; // mesh variation index
   DestructMesh *destruct_mesh ; // pointer to pre-generated destructible object which will be used to spawn debris when destroyed
   Memc<Actor>   actors        ; // actors
   Memc<Joint>   joints        ; // physical joints between actors

   // manage
   virtual void create(Object &obj); // create from object

   virtual void setUnsavedParams(); // set parameters which are not included in the save file

   // get / set
   virtual Vec    pos         (                ); // get position
   virtual void   pos         (C Vec    &pos   ); // set position
   virtual Matrix matrix      (                ); // get matrix  , returned matrix is normalized
   virtual Matrix matrixScaled(                ); // get matrix  , returned matrix is scaled by 'scale'
   virtual void   matrix      (C Matrix &matrix); // set matrix  , 'matrix' must be normalized

   // operations
   virtual void toStatic   (); // convert BREAKABLE           into      STATIC
   virtual void toBreakable(); // convert STATIC              into      BREAKABLE
   virtual void toPieces   (); // convert STATIC or BREAKABLE into many PIECE objects

   // callbacks
   virtual void memoryAddressChanged(); // called when object memory address has been changed, you should override it and adjust Actor::obj pointer for all actors

   virtual void linkReferences(); // this method is called after loading all game objects, you should override it and call 'link' on every Reference that is referencing external objects

   // update
   virtual Bool update(); // update, return false when object wants to be deleted, and true if wants to exist

   // draw
   virtual UInt drawPrepare(); // prepare for drawing, this will be called in RM_PREPARE mode, in this method you should add the meshes to the draw list (Mesh::draw), and return a combination of bits of which additional render modes will be required for custom drawing of the object (for example "return IndexToFlag(RM_BLEND)" requests blend mode rendering)
   virtual void drawShadow (); // in this method you should add the meshes to the shadow draw list (Mesh::drawShadow)

   // enable / disable
   virtual void disable(); // disable object when becomes too far away, here all dynamic actors should  enable kinematic flag - actor.kinematic(true ), this is called when an object changes its state to AREA_INACTIVE
   virtual void  enable(); //  enable object when closes in           , here all dynamic actors should disable kinematic flag - actor.kinematic(false), this is called when an object changes its state to AREA_ACTIVE

   // io
   virtual Bool canBeSaved(       ); // if object wants to be saved, you can override this method and return false when object is useless and doesn't need to be stored in a SaveGame
   virtual Bool save      (File &f); // save, false on fail
   virtual Bool load      (File &f); // load, false on fail

  ~Destructible();
   Destructible();

protected:
   void setPieces   (Bool create_joints);
   void setStatic   (C Matrix &matrix, Byte actor_group);
   void setBreakable(C Matrix &matrix, Byte actor_group);
};
/******************************************************************************/
} // namespace
/******************************************************************************/
