/******************************************************************************/
/******************************************************************************/
class ElmSkel : ElmData
{
   UID       mesh_id;
   Pose      transform;
   TimeStamp file_time;

   // get
   bool equal(C ElmSkel &src)C;
   bool newer(C ElmSkel &src)C;

   virtual bool mayContain (C UID &id)C override;
   virtual void clearLinked(         )  override;

   // operations
   virtual void newData()override;
   uint undo(C ElmSkel &src);
   uint sync(C ElmSkel &src);
   bool syncFile(C ElmSkel &src);

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;

public:
   ElmSkel();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
