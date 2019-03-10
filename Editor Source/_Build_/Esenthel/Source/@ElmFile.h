/******************************************************************************/
/******************************************************************************/
class ElmFile : ElmData
{    
   TimeStamp file_time;

   // get
   bool equal(C ElmFile &src)C;
   bool newer(C ElmFile &src)C;

   virtual bool mayContain(C UID &id)C override;

   // operations
   virtual void newData()override;
   uint undo(C ElmFile &src);
   uint sync(C ElmFile &src);
   bool syncFile(C ElmFile &src);

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
