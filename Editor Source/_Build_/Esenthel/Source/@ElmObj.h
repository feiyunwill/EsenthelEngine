/******************************************************************************/
/******************************************************************************/
class ElmObj : ElmObjClass
{
   UID mesh_id, base_id; // 'base_id' should not be processed in following methods, but only during syncing (it can point to both ElmObj and ElmObjClass)

   // get
   bool equal(C ElmObj &src)C;
   bool newer(C ElmObj &src)C;

   // operations
   virtual void clearLinked()override;

   uint undo(C ElmObj &src);
   uint sync(C ElmObj &src);
   void from(C EditObject &params);

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;

public:
   ElmObj();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
