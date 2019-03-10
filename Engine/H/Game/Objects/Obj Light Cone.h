/******************************************************************************/
namespace Game{
/******************************************************************************/
STRUCT(ObjLightCone , Obj) // Game Cone Light Object
//{
   Bool cast_shadows;
   Flt  range, volumetric, angle, falloff;
   Vec  position, direction, color;

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
   virtual void drawPrepare(C Matrix &matrix); // draw at 'matrix' matrix
   virtual UInt drawPrepare(                ); // prepare for drawing, this will be called in RM_PREPARE mode, in this method you should add the meshes to the draw list (Mesh::draw), and return a combination of bits of which additional render modes will be required for custom drawing of the object (for example "return IndexToFlag(RM_BLEND)" requests blend mode rendering)

   // io
   virtual Bool save(File &f); // save, false on fail
   virtual Bool load(File &f); // load, false on fail

  ~ObjLightCone();
   ObjLightCone();
};
/******************************************************************************/
} // namespace
/******************************************************************************/
