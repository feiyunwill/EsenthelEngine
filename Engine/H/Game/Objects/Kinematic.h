/******************************************************************************

   Use 'Kinematic' class for solid objects, which position can be changed only manually.

/******************************************************************************/
namespace Game{
/******************************************************************************/
STRUCT(Kinematic , Obj) // Game Kinematic Object, these objects are created as kinematic actors, which means that their position is unaffected by other objects, but can be changed manually (this class is similar to Game::Static, with the exception that Static can't move at all)
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
   virtual Vec    pos            (                ); // get     position                                        (based on 'actor.pos            ')
   virtual void   pos            (C Vec    &pos   ); // set     position                                        (based on 'actor.pos            ')
   virtual Matrix matrix         (                ); // get     matrix  , returned matrix is normalized       , (based on 'actor.matrix         ')
   virtual Matrix matrixScaled   (                ); // get     matrix  , returned matrix is scaled by 'scale', (based on 'actor.matrix         ')
   virtual void   matrix         (C Matrix &matrix); // set     matrix  , 'matrix' must be normalized           (based on 'actor.matrix         ')
   virtual void   kinematicMoveTo(C Vec    &pos   ); // move to position                                        (based on 'actor.kinematicMoveTo'), for differences between methods please check comments on 'Actor::kinematicMoveTo'
   virtual void   kinematicMoveTo(C Matrix &matrix); // move to matrix  , 'matrix' must be normalized           (based on 'actor.kinematicMoveTo'), for differences between methods please check comments on 'Actor::kinematicMoveTo'

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

  ~Kinematic();
   Kinematic();

protected:
   Matrix _matrix, _matrix_scaled;
};
/******************************************************************************/
} // namespace
/******************************************************************************/
