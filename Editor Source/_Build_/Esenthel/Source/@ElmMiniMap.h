/******************************************************************************/
/******************************************************************************/
class ElmMiniMap : ElmData
{
   int       areas_per_image, image_size;
   UID       world_id, env_id; // if environment is zero then world default env is used
   TimeStamp areas_per_image_time, image_size_time, world_time, env_time;

   bool equal(C ElmMiniMap &src)C;
   bool newer(C ElmMiniMap &src)C;

   virtual bool mayContain(C UID &id)C override;

   // operations
   virtual void newData()override;

   uint undo(C ElmMiniMap &src);
   uint sync(C ElmMiniMap &src);
   void copyTo(Game::MiniMap::Settings &settings)C;

   // io
   virtual bool save(File &f)C override;
   virtual bool load(File &f)override;
   virtual void save(MemPtr<TextNode> nodes)C override;
   virtual void load(C MemPtr<TextNode> &nodes)override;

public:
   ElmMiniMap();
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
