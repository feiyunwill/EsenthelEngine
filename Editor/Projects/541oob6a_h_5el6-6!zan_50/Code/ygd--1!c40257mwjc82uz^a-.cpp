/******************************************************************************/
class BlocksMapEx : BlocksMap // create a custom class that extends 'BlocksMap' and return there your blocks based on its coordinates in the map
{
   virtual Blocks* findBlocks(Int x, Int y)C
   {
      if(x==0 && y==0)return &blocks;
      return null;
   }
}
/******************************************************************************/
Blocks          blocks;
BlocksOcclusion blocks_occlusion; // helper object used for calculating blocks ambient occlusion
BlocksMapEx     blocks_map;
/******************************************************************************/
void InitPre()
{
   EE_INIT();
   App.flag=APP_MAXIMIZABLE|APP_MINIMIZABLE;

   Ms.hide();
   Ms.clip(null, 1);

   Cam.dist=10;
   Cam.at.set(4, 4, 4);

   Sky.atmospheric();
}
/******************************************************************************/
bool Init()
{
   blocks_map      .create(8, MatrixIdentity); // create blocks map with blocks of 8x8 XZ resolution, Y dimension will have no limit
   blocks          .create(blocks_map.resolution()); // create blocks resolution from the map
   blocks_occlusion.create(2); // create blocks occlusion with 2 blocks range

   // initialize blocks with random settings
   REPD(x, 8)
   REPD(y, 8)
   REPD(z, 8)blocks.set(x, y, z, MaterialPtr(Random(2) ? UID(2123216029, 1141820639, 615850919, 3316401700) : UIDZero)); // set Brick or null material

   // set blocks mesh
   blocks.setMesh(1.0, &blocks_occlusion, null, VecI2(0, 0), &blocks_map);

   return true;
}
/******************************************************************************/
void Shut()
{
}
/******************************************************************************/
bool Update()
{
   if(Kb.bp(KB_ESC))return false;
   Cam.transformByMouse(0.01, 50, CAMH_ZOOM|(Ms.b(1) ? CAMH_MOVE : CAMH_ROT));
   
   // dynamically adjust the blocks
   Int x=Random(8),
       y=Random(8),
       z=Random(8);
   blocks.set(x, y, z, MaterialPtr(Random(2) ? UID(2123216029, 1141820639, 615850919, 3316401700) : UIDZero)); // set Brick or null material

   // update blocks mesh, this time we'll just update parts that were changed by specifying 'local_box' parameter
   blocks.setMesh(1.0, &blocks_occlusion, &BoxI(VecI(x, y, z)), VecI2(0, 0), &blocks_map);

   return true;
}
/******************************************************************************/
void Render()
{
   switch(Renderer())
   {
      case RM_PREPARE:
      {
         blocks.draw(blocks_map.matrix(0, 0)); // draw blocks with its matrix

         LightDir(Cam.matrix.z, 1-D.ambientColor()).add();
      }break;
   }
}
/******************************************************************************/
void Draw()
{
   Renderer(Render);
}
/******************************************************************************/
