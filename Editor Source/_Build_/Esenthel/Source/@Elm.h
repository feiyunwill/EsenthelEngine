/******************************************************************************/
/******************************************************************************/
class Elm
{
   enum FLAG // !! these enums are saved !!
   {
      IMPORTING       =1<<0,
      OPENED          =1<<1,
      REMOVED         =1<<2,
      NO_PUBLISH      =1<<3,
      FINAL_REMOVED   =1<<4,
      FINAL_NO_PUBLISH=1<<5,
      DATA            =1<<7, // used only in IO
   };
   ELM_TYPE        type;
   byte                   flag; // FLAG
   int              file_size; // game file size, -1=unknown !! this is not saved !!
   UID id, // id   of the element
               parent_id; // id   of the parent
   Str                      name; // name of the element
   TimeStamp       //create_time, // time when the element was created
                       name_time, // time when the element was named or renamed
                     parent_time, // time when the element was attached to the parent
                    removed_time, // time when the element was removed or restored
                 no_publish_time; // time when the element had NO_PUBLISH changed
   ElmData            *data;

  ~Elm();

   Elm& copyParams(C Elm &src, bool set_parent=true); // copy parameters from 'src', 'type' must be already specified, doesn't copy ID
   void operator=(C Elm &src);
   Elm(C Elm &src);        

   // get
   bool importing     ()C;   Elm& importing     (bool on);
   bool opened        ()C;   Elm& opened        (bool on);
   bool removed       ()C;   Elm& removed       (bool on); // this checks only if this element is    removed, it doesn't check the parents
   bool publish       ()C;   Elm& publish       (bool on); // this checks only if this element is    publish, it doesn't check the parents
   bool noPublish     ()C;   Elm& noPublish     (bool on); // this checks only if this element is no publish, it doesn't check the parents
   bool finalRemoved  ()C;   Elm& finalRemoved  (bool on);
   bool finalExists   ()C;   Elm& finalExists   (bool on);
   bool finalPublish  ()C;   Elm& finalPublish  (bool on); // this includes 'finalExists'  as well !!
   bool finalNoPublish()C;   Elm& finalNoPublish(bool on); // this includes 'finalRemoved' as well !!
 C Str& srcFile       ()C;
   bool initialized   ()C;

   void resetFinal();

   Elm& setRemoved  (  bool removed   , C TimeStamp &time=TimeStamp().getUTC());
   Elm& setNoPublish(  bool no_publish, C TimeStamp &time=TimeStamp().getUTC());
   Elm& setName     (C Str &name      , C TimeStamp &time=TimeStamp().getUTC());
   Elm& setParent   (C UID &parent_id , C TimeStamp &time=TimeStamp().getUTC());
   Elm& setParent   (C Elm *parent    , C TimeStamp &time=TimeStamp().getUTC());
   Elm& setSrcFile  (C Str &src_file  , C TimeStamp &time=TimeStamp().getUTC());

   ElmObjClass  *   objClassData();   C ElmObjClass  *   objClassData()C;
   ElmObj       *        objData();   C ElmObj       *        objData()C;
   ElmMesh      *       meshData();   C ElmMesh      *       meshData()C;
   ElmMaterial  *       mtrlData();   C ElmMaterial  *       mtrlData()C;
   ElmWaterMtrl *  waterMtrlData();   C ElmWaterMtrl *  waterMtrlData()C;
   ElmPhysMtrl  *   physMtrlData();   C ElmPhysMtrl  *   physMtrlData()C;
   ElmSkel      *       skelData();   C ElmSkel      *       skelData()C;
   ElmPhys      *       physData();   C ElmPhys      *       physData()C;
   ElmAnim      *       animData();   C ElmAnim      *       animData()C;
   ElmPanelImage* panelImageData();   C ElmPanelImage* panelImageData()C;
   ElmEnv       *        envData();   C ElmEnv       *        envData()C;
   ElmWorld     *      worldData();   C ElmWorld     *      worldData()C;
   ElmMiniMap   *    miniMapData();   C ElmMiniMap   *    miniMapData()C;
   ElmEnum      *       enumData();   C ElmEnum      *       enumData()C;
   ElmImage     *      imageData();   C ElmImage     *      imageData()C;
   ElmImageAtlas* imageAtlasData();   C ElmImageAtlas* imageAtlasData()C;
   ElmIconSetts *  iconSettsData();   C ElmIconSetts *  iconSettsData()C;
   ElmIcon      *       iconData();   C ElmIcon      *       iconData()C;
   ElmFont      *       fontData();   C ElmFont      *       fontData()C;
   ElmTextStyle *  textStyleData();   C ElmTextStyle *  textStyleData()C;
   ElmPanel     *      panelData();   C ElmPanel     *      panelData()C;
   ElmGuiSkin   *    guiSkinData();   C ElmGuiSkin   *    guiSkinData()C;
   ElmGui       *        guiData();   C ElmGui       *        guiData()C;
   ElmSound     *      soundData();   C ElmSound     *      soundData()C;
   ElmVideo     *      videoData();   C ElmVideo     *      videoData()C;
   ElmFile      *       fileData();   C ElmFile      *       fileData()C;
   ElmCode      *       codeData();   C ElmCode      *       codeData()C;
   ElmApp       *        appData();   C ElmApp       *        appData()C;
   ElmData      *           Data();

   bool newerFile(C Elm &src)C; // if 'this' has newer file than 'src'
   bool newerData(C Elm &src)C; // if 'this' has any data newer than 'src'
   uint syncData(C Elm &src);

   // io
   bool save(File &f, bool network, bool skip_name_data)C;
   bool load(File &f, bool network, bool skip_name_data);
   void save(TextNode &node)C;
   bool load(C TextNode &node, Str &error); // assumes that 'error' doesn't need to be cleared at start, and 'T.id' was already set

   void compressNew(File &f)C;
   void decompressNew(File &f);

   void compressData(File &f)C;
   bool decompressData(File &f);

public:
   Elm();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
