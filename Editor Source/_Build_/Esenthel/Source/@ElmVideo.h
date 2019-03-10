/******************************************************************************/
/******************************************************************************/
class ElmVideo : ElmData
{    
   TimeStamp file_time;

   // get
   bool equal(C ElmVideo &src)C;
   bool newer(C ElmVideo &src)C;

   virtual bool mayContain(C UID &id)C override;

   // operations
   virtual void newData()override;
   uint undo(C ElmVideo &src);
   uint sync(C ElmVideo &src);
   bool syncFile(C ElmVideo &src);

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
