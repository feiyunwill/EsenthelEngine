/******************************************************************************/
/******************************************************************************/
class EditorServer : Edit::EditorServer
{
   bool busy, received;
   uint start_time;

   static void ConvertHeight(C Heightmap &src, Image &dest, flt area_size);
   static void ConvertHeight(Image &src, Heightmap &dest, flt area_size); // 'src' will get modified !!
   static void ConvertColor(C Heightmap &src, Image &dest);
   static void ConvertColor(Image &src, Heightmap &dest); // 'src' will get modified !!
   static void ConvertMaterial(C Heightmap &src, Edit::MaterialMap &dest);
   static void ConvertMaterial(Edit::MaterialMap &src, Heightmap &dest); // 'src' will get modified !!

   class Client : Edit::EditorServer::Client
   {
      int queued; // queued command to be processed

      virtual bool update()override;

public:
   Client();
   };

   void update(bool busy);

   EditorServer();
};
/******************************************************************************/
/******************************************************************************/
extern EditorServer EditServer;
/******************************************************************************/
