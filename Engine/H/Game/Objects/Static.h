/******************************************************************************

   Use 'Static' class for solid objects, which position will never change.

/******************************************************************************/
namespace Game{
/******************************************************************************/
STRUCT(Static , Obj) // Game Static Object, these objects are static and can't be moved during game (this class is similar to Game::Kinematic, with the exception that Kinematic can be moved manually)
//{
   Vec       scale         ; // scale
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

   // callbacks
   virtual void memoryAddressChanged(); // called when object memory address has been changed, you should override it and adjust Actor::obj pointer for all actors

   // update
   virtual Bool update(); // update, return false when object wants to be deleted, and true if wants to exist

   // draw
   virtual UInt drawPrepare(); // prepare for drawing, this will be called in RM_PREPARE mode, in this method you should add the meshes to the draw list (Mesh::draw), and return a combination of bits of which additional render modes will be required for custom drawing of the object (for example "return IndexToFlag(RM_BLEND)" requests blend mode rendering)
   virtual void drawShadow (); // in this method you should add the meshes to the shadow draw list (Mesh::drawShadow)

   // io
   virtual Bool save(File &f); // save, false on fail
   virtual Bool load(File &f); // load, false on fail

  ~Static();
   Static();

protected:
   Matrix _matrix, _matrix_scaled;
};
/******************************************************************************/
} // namespace
/******************************************************************************/
