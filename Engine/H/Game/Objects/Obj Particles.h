/******************************************************************************/
namespace Game{
/******************************************************************************/
STRUCT(ObjParticles , Obj) // Game Particles Object
//{
   Particles particles;

   // manage
   virtual void create(Object &obj); // create from object

   // get / set
   virtual Vec    pos   (                ); // get position
   virtual void   pos   (C Vec    &pos   ); // set position
   virtual Matrix matrix(                ); // get matrix
   virtual void   matrix(C Matrix &matrix); // set matrix

   // update
   virtual Bool update(); // update, return false when object wants to be deleted, and true if wants to exist

   // draw
   virtual UInt drawPrepare (); // prepare for drawing, this will be called in RM_PREPARE mode, in this method you should add the meshes to the draw list (Mesh::draw), and return a combination of bits of which additional render modes will be required for custom drawing of the object (for example "return IndexToFlag(RM_BLEND)" requests blend mode rendering)
   virtual void drawBlend   (); // in this method you should draw custom objects for the RM_BLEND    mode, this method will be called only if the 'drawPrepare' has requested RM_BLEND    mode
   virtual void drawPalette (); // in this method you should draw custom objects for the RM_PALETTE  mode, this method will be called only if the 'drawPrepare' has requested RM_PALETTE  mode
   virtual void drawPalette1(); // in this method you should draw custom objects for the RM_PALETTE1 mode, this method will be called only if the 'drawPrepare' has requested RM_PALETTE1 mode

   // io
   virtual Bool save(File &f); // save, false on fail
   virtual Bool load(File &f); // load, false on fail

  ~ObjParticles();
   ObjParticles();
};
/******************************************************************************/
} // namespace
/******************************************************************************/
