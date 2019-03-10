/******************************************************************************

   Use 'Door' class for creating door objects.

/******************************************************************************/
namespace Game{
/******************************************************************************/
STRUCT(Door , Obj) // Game Door Object
//{
   Flt          scale         ; // scale
   ObjectPtr    base          ; // base object
   MeshPtr      mesh          ; // mesh
   Int          mesh_variation; // mesh variation index
   Actor        actor         ; // actor
   Joint        joint         ; // joint
   PathObstacle obstacle      ; // path mesh obstacle

   // manage
   virtual void create(Object &obj); // create from object

   virtual void setUnsavedParams(); // set parameters which are not included in the save file

   // get / set
   virtual Vec    pos   (                );   // get position
   virtual Matrix matrix(                );   // get matrix
   virtual void   pos   (C Vec    &pos   ) {} // set position, Door objects have this method disabled
   virtual void   matrix(C Matrix &matrix) {} // set matrix  , Door objects have this method disabled

   // callbacks
   virtual void memoryAddressChanged(); // called when object memory address has been changed, you should override it and adjust Actor::obj pointer for all actors

   // update
   virtual Bool update(); // update, return false when object wants to be deleted, and true if wants to exist

   // draw
   virtual UInt drawPrepare(); // prepare for drawing, this will be called in RM_PREPARE mode, in this method you should add the meshes to the draw list (Mesh::draw), and return a combination of bits of which additional render modes will be required for custom drawing of the object (for example "return IndexToFlag(RM_BLEND)" requests blend mode rendering)
   virtual void drawShadow (); // in this method you should add the meshes to the shadow draw list (Mesh::drawShadow)

   // enable / disable
   virtual void disable(); // disable object when becomes too far away, here all dynamic actors should  enable kinematic flag - actor.kinematic(true ), this is called when an object changes its state to AREA_INACTIVE
   virtual void  enable(); //  enable object when closes in           , here all dynamic actors should disable kinematic flag - actor.kinematic(false), this is called when an object changes its state to AREA_ACTIVE

   // manipulate
   virtual void open  (); // open   door
   virtual void close (); // close  door
   virtual void toggle(); // toggle door

   // io
   virtual Bool save(File &f); // save, false on fail
   virtual Bool load(File &f); // load, false on fail

  ~Door();
   Door();

protected:
   enum STATE
   {
      STATE_CLOSED ,
      STATE_OPENED ,
      STATE_UNKNOWN,
   };
   Bool   _open;
   Byte   _state;
   Flt    _angle;
   Flt    _actor_to_hinge_dist  ; // actor to hinge distance
   Matrix          _hinge_matrix; // hinge matrix

   void setObstacle(); // set path 'obstacle' basing on the door state
};
/******************************************************************************/
} // namespace
/******************************************************************************/
