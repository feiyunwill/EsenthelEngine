/******************************************************************************/
class ElmPanel : ElmData
{
   Mems<UID> image_ids;

   virtual bool mayContain(C UID &id)C override;

   // operations
   void from(C EditPanel &panel);
   uint undo(C ElmPanel &src);  // don't adjust 'ver' here because it also relies on 'EditPanel', because of that this is included in 'ElmFileInShort'
   uint sync(C ElmPanel &src);  // don't adjust 'ver' here because it also relies on 'EditPanel', because of that this is included in 'ElmFileInShort'

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
