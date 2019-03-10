/******************************************************************************/
/******************************************************************************/
class ObjData // World Object
{
   bool       removed;
   UID        id; // Instance ID
   Matrix     matrix;
   EditObject params;
   TimeStamp  matrix_time, removed_time;

   // get
   bool old(C TimeStamp &now=TimeStamp().getUTC())C;

   bool equal(C ObjData &src)C;
   bool newer(C ObjData &src)C;

   TerrainObj terrainObj()C;
   PhysPath   physPath  ()C;

   uint memUsage()C;

   // operations
   void setRemoved(bool removed, C TimeStamp &time=TimeStamp().getUTC());

   void create(C ObjData &src);
   bool sync(C ObjData &src, cchar *edit_path);
   bool undo(C ObjData &src, cchar *edit_path);
   void create(C Game::Area::Data::AreaObj &src, C UID &type, C EditObjectPtr &base, C TimeStamp &time=TimeStamp().getUTC());
   void copyTo(Game::Area::Data::AreaObj &dest, C Project &proj)C;
   ObjData& create(C Edit::WorldObjParams &src, C Str &edit_path);

   // io
   bool save(File &f, cchar *edit_path=null)C;
   bool load(File &f, cchar *edit_path=null);

public:
   ObjData();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
