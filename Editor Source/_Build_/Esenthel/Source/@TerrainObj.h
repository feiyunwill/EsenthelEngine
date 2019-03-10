/******************************************************************************/
/******************************************************************************/
class TerrainObj // Terrain Object for Embed Generation
{
   EditObjectPtr obj; // object

   operator bool()C;

   bool operator==(C TerrainObj &to)C;
   bool operator!=(C TerrainObj &to)C;

   void set      (OBJ_ACCESS access, C EditObjectPtr &obj);
   void operator=(C ObjData &obj                         );

   TerrainObj(OBJ_ACCESS access, C EditObjectPtr &obj);
   TerrainObj(C ObjData &obj                         );
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
