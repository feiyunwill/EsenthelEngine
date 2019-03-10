/******************************************************************************/
/******************************************************************************/
class ElmEnv : ElmData
{
   UID sun_id, star_id, skybox_id, cloud_id[4];

   ElmEnv();

   // operations
   void from(C EditEnv &src);
   uint undo(C ElmEnv &src);  // don't adjust 'ver' here because it also relies on 'EditEnv', because of that this is included in 'ElmFileInShort'
   uint sync(C ElmEnv &src);  // don't adjust 'ver' here because it also relies on 'EditEnv', because of that this is included in 'ElmFileInShort'

   virtual bool mayContain(C UID &id)C override;

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
