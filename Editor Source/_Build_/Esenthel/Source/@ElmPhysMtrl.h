/******************************************************************************/
/******************************************************************************/
class ElmPhysMtrl : ElmData
{
   // get
   bool equal(C ElmPhysMtrl &src)C;
   bool newer(C ElmPhysMtrl &src)C;

   virtual bool mayContain(C UID &id)C override;

   // operations
   void from(C EditPhysMtrl &src);
   uint undo(C  ElmPhysMtrl &src); // don't adjust 'ver' here because it also relies on 'EditPhysMtrl', because of that this is included in 'ElmFileInShort'
   uint sync(C  ElmPhysMtrl &src); // don't adjust 'ver' here because it also relies on 'EditPhysMtrl', because of that this is included in 'ElmFileInShort'

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
