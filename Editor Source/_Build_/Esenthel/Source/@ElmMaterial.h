/******************************************************************************/
/******************************************************************************/
class ElmMaterial : ElmData
{
   enum FLAG
   {
      USES_TEX_ALPHA =1<<0,
      USES_TEX_BUMP  =1<<1,
      USES_TEX_GLOW  =1<<2,
      TEX_QUALITY_IOS=1<<3,
   };
   UID  base_0_tex, base_1_tex, detail_tex, macro_tex, reflection_tex, light_tex;
   byte downsize_tex_mobile, flag;

   // get
   bool equal(C ElmMaterial &src)C;
   bool newer(C ElmMaterial &src)C;

   bool usesTexAlpha ()C;   void usesTexAlpha (bool on);
   bool usesTexBump  ()C;   void usesTexBump  (bool on);
   bool usesTexGlow  ()C;   void usesTexGlow  (bool on);
   bool texQualityiOS()C;   void texQualityiOS(bool on);

   virtual bool mayContain (C UID &id)C override;               
   virtual bool containsTex(C UID &id, bool test_merged)C override;
   virtual void listTexs(MemPtr<UID> texs)C override;

   // operations
   void from(C EditMaterial &src);
   uint undo(C ElmMaterial &src); // don't undo 'downsize_tex_mobile', 'flag' because they should be set only in 'from'
   uint sync(C ElmMaterial &src); // don't sync 'downsize_tex_mobile', 'flag' because they should be set only in 'from'

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;

public:
   ElmMaterial();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
