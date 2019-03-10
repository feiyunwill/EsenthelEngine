/******************************************************************************/
/******************************************************************************/
class ElmPhys : ElmData
{
   UID       mesh_id, mtrl_id;
   flt       density;
   Pose      transform;
   Box       box; // this should not be synced (it is set according to phys data), set as invalid when phys is empty
   TimeStamp file_time, mtrl_time, density_time;

   // get
   bool hasBody()C;

   bool equal(C ElmPhys &src)C;
   bool newer(C ElmPhys &src)C;

   virtual bool mayContain (C UID &id)C override;
   virtual void clearLinked(         )  override;

   // operations
   virtual void newData()override;
   uint undo(C ElmPhys &src);
   uint sync(C ElmPhys &src);
   bool syncFile(C ElmPhys &src);
   void from(C PhysBody &game_phys); // game version of phys must be passed here

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;

public:
   ElmPhys();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
