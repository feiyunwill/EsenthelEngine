/******************************************************************************/
/******************************************************************************/
class ElmData // !! IMPORTANT: all classes extending 'ElmData' must override 'undo' and 'sync' methods, and adjust 'ver' !!
{
   Version   ver; // value randomized each time the data is changed indicating that client/server sync is needed
   Str       src_file; // can have multiple files and parameters, encoded using 'Edit.FileParams'
   TimeStamp src_file_time;

   // get
   bool equal(C ElmData &src)C;
   bool newer(C ElmData &src)C;

   virtual bool mayContain (C UID &id)C;              
   virtual bool containsTex(C UID &id, bool test_merged)C;
   virtual void listTexs   (MemPtr<UID> texs)C;  

   // operations
           void newVer ();                  
   virtual void newData();                           
   void  setSrcFile(C Str &src_file, C TimeStamp&time=TimeStamp().getUTC());
   uint undoSrcFile(C ElmData &src);                            
   uint undo(C ElmData &src);
   uint sync(C ElmData &src);
   virtual void clearLinked(); // this should clear the specially linked objects (such as obj->mesh, mesh->skel,phys, skel->mesh, phys->mesh)

   // io
   virtual bool save(File &f)C;
   virtual bool load(File &f);
   virtual void save(MemPtr<TextNode> nodes)C;
   virtual void load(C MemPtr<TextNode> &nodes);

   virtual ~ElmData();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
