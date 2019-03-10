/******************************************************************************/
/******************************************************************************/
class ElmImage : ElmData
{
   enum FLAG // !! these enums are saved !!
   {
      MIP_MAPS =1<<0,
      POW2     =1<<1,
      ALPHA_LUM=1<<2,
      HAS_COLOR=1<<3, // if image is not monochromatic (r!=g || r!=b) (this member is not synced, it is inherited from image data)
      HAS_ALPHA=1<<4, // if any alpha pixel is not 255                (this member is not synced, it is inherited from image data)
   };
   enum TYPE : byte // !! these enums are saved !!
   {
      COMPRESSED ,
      COMPRESSED2,
      FULL       ,
      ALPHA      ,
    //CUSTOM     ,
      NUM        ,
   };
   static NameDesc ImageTypes[]
;
   static int ImageTypesElms;
   class ImageMode
   {
      IMAGE_MODE mode;
      cchar8    *name;
   };
   static ImageMode ImageModes[]
;

   byte       flag; // FLAG
   TYPE       type;
   IMAGE_MODE mode;
   VecI2      size; // -1=use existing, 0=default (use existing but adjust scale to keep aspect ratio if other value is modified)
   TimeStamp  mip_maps_time, pow2_time, alpha_lum_time, type_time, mode_time, size_time, file_time;

   bool       ignoreAlpha()C;
   bool       mipMaps    ()C;   void mipMaps (bool on);
   bool       pow2       ()C;   void pow2    (bool on);
   bool       alphaLum   ()C;   void alphaLum(bool on);
   bool       hasColor   ()C;   void hasColor(bool on);
   bool       hasAlpha   ()C;   void hasAlpha(bool on);
   bool       hasAlpha2  ()C;
   bool       hasAlpha3  ()C;
   IMAGE_TYPE androidType()C; // if want to be compressed then use ETC2_A8 or ETC2 or ETC1
   IMAGE_TYPE     iOSType()C; // if want to be compressed then use PVRTC1_4
   IMAGE_TYPE     webType()C; // if want to be compressed with alpha then use BC3

   // get
   bool equal(C ElmImage &src)C;
   bool newer(C ElmImage &src)C;

   virtual bool mayContain(C UID &id)C override;

   // operations
   virtual void newData()override;
   uint undo(C ElmImage &src);
   uint sync(C ElmImage &src);
   bool syncFile(C ElmImage &src);

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;

public:
   ElmImage();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
