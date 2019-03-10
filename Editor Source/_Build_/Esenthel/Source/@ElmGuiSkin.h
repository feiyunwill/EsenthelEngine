/******************************************************************************/
/******************************************************************************/
class ElmGuiSkin : ElmData
{
   Mems<UID> elm_ids;

   virtual bool mayContain(C UID &id)C override;

   // operations
   void from(C EditGuiSkin &pi);
   uint undo(C ElmGuiSkin &src); // don't adjust 'ver' here because it also relies on 'EditGuiSkin', because of that this is included in 'ElmFileInShort'
   uint sync(C ElmGuiSkin &src); // don't adjust 'ver' here because it also relies on 'EditGuiSkin', because of that this is included in 'ElmFileInShort'

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
