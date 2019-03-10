/******************************************************************************

   Use 'Item' as a base class for creating your own item class, in order to add more custom parameters.

/******************************************************************************/
namespace Game{
/******************************************************************************/
STRUCT(Item , Obj) // Game Item Object
//{
   Flt       scale         ; // scale
   ObjectPtr base          ; // base object
   MeshPtr   mesh          ; // mesh
   Int       mesh_variation; // mesh variation index
   Actor     actor         ; // actor

   // manage
   virtual void create(Object &obj); // create from object

   virtual void setUnsavedParams(); // set parameters which are not included in the save file

   // get / set
   virtual Vec    pos         (                ); // get position
   virtual void   pos         (C Vec    &pos   ); // set position
   virtual Matrix matrix      (                ); // get matrix  , returned matrix is normalized
   virtual Matrix matrixScaled(                ); // get matrix  , returned matrix is scaled by 'scale'
   virtual void   matrix      (C Matrix &matrix); // set matrix  , 'matrix' must be normalized

   void setDrawingVelocities(C Vec &vel, C Vec &ang_vel); // manually set drawing velocities for motion blur effect, this method should be called for items in the inventory which have their actor deactivated, but still want to be drawn, this shouldn't be called before 'update' because 'update' sets the default velocities from the actor

   // callbacks
   virtual void memoryAddressChanged(); // called when object memory address has been changed, you should override it and adjust Actor::obj pointer for all actors

   virtual void willBeMovedFromWorldToStorage(); // called before object was moved from world                   to custom object container
   virtual void willBeMovedFromStorageToWorld(); // called before object was moved from custom object container to world

   // update
   virtual Bool update(); // update, return false when object wants to be deleted, and true if wants to exist

   // draw
   virtual UInt drawPrepare(); // prepare for drawing, this will be called in RM_PREPARE mode, in this method you should add the meshes to the draw list (Mesh::draw), and return a combination of bits of which additional render modes will be required for custom drawing of the object (for example "return IndexToFlag(RM_BLEND)" requests blend mode rendering)
   virtual void drawShadow (); // in this method you should add the meshes to the shadow draw list (Mesh::drawShadow)

   // enable / disable
   virtual void disable(); // disable object when becomes too far away, here all dynamic actors should  enable kinematic flag - actor.kinematic(true ), this is called when an object changes its state to AREA_INACTIVE
   virtual void  enable(); //  enable object when closes in           , here all dynamic actors should disable kinematic flag - actor.kinematic(false), this is called when an object changes its state to AREA_ACTIVE

   // io
   virtual Bool save(File &f); // save, false on fail
   virtual Bool load(File &f); // load, false on fail

  ~Item();
   Item();

public: // following members/methods are public, however don't modify/call them manually unless you know what you're doing
   Chr* grabber(        ) {return _grabber    ;} // get character grabbing the item
   void grabber(Chr *chr) {       _grabber=chr;} // set character grabbing the item

protected:
   Chr *_grabber;
   Vec  _vel, _ang_vel;
};
/******************************************************************************/
} // namespace
/******************************************************************************/
