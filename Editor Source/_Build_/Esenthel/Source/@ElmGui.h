/******************************************************************************/
class ElmGui : ElmData
{
   TimeStamp file_time;

   // get
   bool equal(C ElmGui &src)C;
   bool newer(C ElmGui &src)C;

   // operations
   virtual void newData()override;
   uint undo(C ElmGui &src);
   uint sync(C ElmGui &src);
   bool syncFile(C ElmGui &src);

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
