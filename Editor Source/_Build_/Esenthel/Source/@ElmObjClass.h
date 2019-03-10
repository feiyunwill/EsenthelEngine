/******************************************************************************/
/******************************************************************************/
class ElmObjClass : ElmData
{
   enum FLAG // !! these enums are saved !!
   {
      OVR_ACCESS=1<<0,
      OVR_PATH  =1<<1,
      TERRAIN   =1<<2,
      PATH_SHIFT=   3,
      PATH_MASK = 1|2, // use 2 bits for mask to store up to 4 path modes
   };
   byte flag; // FLAG, this should not be synced, it is set only from data

   // get
   bool     ovrAccess()C;   void ovrAccess(bool     on  );
   bool     terrain  ()C;   void terrain  (bool     on  );
   bool     ovrPath  ()C;   void ovrPath  (bool     on  );
   OBJ_PATH pathSelf ()C;   void pathSelf (OBJ_PATH path);

   // operations
   void from(C EditObject &params);
   uint undo(C ElmObjClass &src); // don't adjust 'ver' here because it also relies on 'EditObject', because of that this is included in 'ElmFileInShort'
   uint sync(C ElmObjClass &src); // don't adjust 'ver' here because it also relies on 'EditObject', because of that this is included in 'ElmFileInShort'

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;

public:
   ElmObjClass();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
