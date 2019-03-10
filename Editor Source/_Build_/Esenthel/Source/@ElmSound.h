/******************************************************************************/
/******************************************************************************/
class ElmSound : ElmData
{    
   TimeStamp file_time;

   // get
   bool equal(C ElmSound &src)C;
   bool newer(C ElmSound &src)C;

   virtual bool mayContain(C UID &id)C override;

   // operations
   virtual void newData()override;
   uint undo(C ElmSound &src);
   uint sync(C ElmSound &src);
   bool syncFile(C ElmSound &src);

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
