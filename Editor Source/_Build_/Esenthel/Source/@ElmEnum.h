/******************************************************************************/
/******************************************************************************/
class ElmEnum : ElmData
{
   EditEnums::TYPE type; // this should not be synced, it is set only from data

   virtual bool mayContain(C UID &id)C override;

   // operations
   void from(C EditEnums &enums);
   uint undo(C  ElmEnum  &src); // don't adjust 'ver' here because it also relies on 'EditEnums', because of that this is included in 'ElmFileInShort'
   uint sync(C  ElmEnum  &src); // don't adjust 'ver' here because it also relies on 'EditEnums', because of that this is included in 'ElmFileInShort'

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;

public:
   ElmEnum();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
