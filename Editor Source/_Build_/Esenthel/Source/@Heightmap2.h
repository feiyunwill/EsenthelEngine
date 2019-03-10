/******************************************************************************/
/******************************************************************************/
class Heightmap2 : Heightmap
{
 //Heightmap server_hm; heightmap before any modifications on the client side (use this in the future for delta-based updating of heightmaps to the server)
   Mesh      mesh;
   PhysPart  phys;
   Image     height_map, mtrl_map;
   Memc<UID> mtrls;

   bool builded()C;

   bool invalidRefs()C;

   void setShader();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
