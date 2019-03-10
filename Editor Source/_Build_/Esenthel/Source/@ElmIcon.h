/******************************************************************************/
/******************************************************************************/
class ElmIcon : ElmData
{
   enum FLAG // !! these enums are saved !!
   {
      HAS_COLOR=1<<0, // if image is not monochromatic (r!=g || r!=b) (this member is not synced, it is inherited from image data)
      HAS_ALPHA=1<<1, // if any alpha pixel is not 255                (this member is not synced, it is inherited from image data)
   };
   static const uint InheritVariation; // use from object in case it overrides default variation

   byte      flag; // FLAG
   UID       icon_settings_id, obj_id, anim_id;
   uint      variation_id;
   flt       anim_pos;
   TimeStamp icon_settings_time, obj_time, file_time, anim_id_time, anim_pos_time, variation_time;

   ElmImage::TYPE     type(Project *proj)C;                          
   bool          hasColor(             )C; ElmIcon&hasColor(bool on);
   bool          hasAlpha(             )C; ElmIcon&hasAlpha(bool on);
   IMAGE_TYPE androidType(Project *proj)C;                          // if want to be compressed then use ETC2_A8 or ETC2 or ETC1
   IMAGE_TYPE     iOSType(Project *proj)C;                          // if want to be compressed then use PVRTC1_4
   IMAGE_TYPE     webType(Project *proj)C;                          // if want to be compressed with alpha then use BC3

   bool equal(C ElmIcon &src)C;
   bool newer(C ElmIcon &src)C;

   virtual bool mayContain(C UID &id)C override;

   // operations
   virtual void newData()override;
   uint undo(C ElmIcon &src);
   uint sync(C ElmIcon &src);
   bool syncFile(C ElmIcon &src);

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;

public:
   ElmIcon();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
