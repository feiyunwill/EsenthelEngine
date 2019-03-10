/******************************************************************************/
class TerrainObj2 // Terrain Object for Embed Generation
{
   MeshPtr     mesh;
   PhysBodyPtr phys;

   operator bool()C;

   bool operator==(C TerrainObj2 &to)C;
   bool operator!=(C TerrainObj2 &to)C;

   void operator=(C Object &obj);

   TerrainObj2();  
   TerrainObj2(C Object &obj);
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
