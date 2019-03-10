/******************************************************************************/
class ElmTextStyle : ElmData
{
   UID font_id;

   virtual bool mayContain(C UID &id)C override;

   // operations
   void from(C EditTextStyle &ts );
   uint undo(C  ElmTextStyle &src); // don't adjust 'ver' here because it also relies on 'EditTextStyle', because of that this is included in 'ElmFileInShort'
   uint sync(C  ElmTextStyle &src); // don't adjust 'ver' here because it also relies on 'EditTextStyle', because of that this is included in 'ElmFileInShort'

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;

public:
   ElmTextStyle();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
